#include "largeList.h"

extern "C" SEXP removeFromList(SEXP file, SEXP index)
{
  //check parameters
  if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("File should be a charater vector of length 1.\n");
  if (index != R_NilValue && TYPEOF(index) != INTSXP &&  TYPEOF(index) != REALSXP && TYPEOF(index) != STRSXP)
    error("Index should be an integer vector, a numeric vector or a character vector.\n");
  
  const char *fileName = getFullPath(file);
  FILE *fin;
  FILE *fout;
  fin = fopen(fileName,"rb");
  fout = fopen(fileName,"rb+");
  std::vector<int64_t> positions;
  std::vector<int> indexNum;
  std::vector<std::string> names;

  //get list length
  fseek(fin, 18, SEEK_SET);
  int lengthOfList;
  fread((char*)&(lengthOfList), 4, 1, fin);

  if (TYPEOF(index) == INTSXP ||  TYPEOF(index) == REALSXP){
    indexNum.resize(Rf_length(index));
    TYPEOF(index) == INTSXP ? 
      indexNum.assign(INTEGER(index),INTEGER(index)+Rf_length(index)) :
      indexNum.assign(REAL(index),REAL(index)+Rf_length(index));
    for(int i = 0 ; i < Rf_length(index); i++){
      indexNum[i] = indexNum[i]-1;
    }
    int maxIndex = *std::max_element(indexNum.begin(), indexNum.end());
    int minIndex = *std::min_element(indexNum.begin(), indexNum.end());
    if (minIndex < 0) error("Index should be positive.");
    if (maxIndex > lengthOfList-1) error("Index beyonds list length.");
  }

  if (TYPEOF(index) == STRSXP){
    names.resize(Rf_length(index));
    for (int i = 0; i < Rf_length(index); i ++){
      names[i].assign(CHAR(STRING_ELT(index,i)),Rf_length(STRING_ELT(index,i)));
    }
    positions.resize(names.size());
    indexNum.resize(names.size());
    for(int i = 0 ; i < Rf_length(index); i++){
      names[i].resize(NAMELENGTH);
      fileBinarySearch(fin,positions[i],names[i],indexNum[i],lengthOfList);
      if (positions[i] == -1)
      {
        Rf_warning("Element with name %s not found! \n",names[i].c_str());
        indexNum[i] =  -1;
      }else{
        fileBinarySearchIndex(fin,positions[i],indexNum[i],lengthOfList);
      }
    }
    //remove invalide elements in the delete list
    int deleteNum = 0;
    for(size_t i = 0 ; i < names.size(); i++){
      if (indexNum[i] ==  -1){
        indexNum.erase(indexNum.begin()+i-deleteNum);
        deleteNum ++;
      }
    }

    //if no element to delete, exit.
    if (indexNum.size() == 0){
      return(ScalarLogical(1));
    }
  }

  std::sort(indexNum.begin(),indexNum.end());
  int64_t moveToPosition;
  int64_t toMoveFirstPosition;
  int64_t toMoveLastPosition;
  int64_t positionDiff = 0;
  int toMoveFirstIndex;
  int toMoveLastIndex;

  //get all positions and names
  fseek(fin, -(8+NAMELENGTH)*2*lengthOfList-8, SEEK_END);
  std::vector<std::pair<std::string, int64_t> > itemIdx(lengthOfList+1);
  readItemIdx(itemIdx, fin, lengthOfList);
  fread((char *)&(itemIdx[lengthOfList].second), 8, 1, fin);

  moveToPosition = itemIdx[indexNum[0]].second;

  //Rprintf("index %d, position %lf", indexNum[0], (double)moveToPosition);
  //move elements
  for (size_t i = 0; i < indexNum.size(); i ++){
    if (indexNum[i] == lengthOfList -1) break; // if the to remove element is the last in the list, do nothing.
    positionDiff += itemIdx[indexNum[i]+1].second - itemIdx[indexNum[i]].second;
    if (i == indexNum.size() -1){
      toMoveFirstIndex = indexNum[i] +1;
      toMoveLastIndex = lengthOfList -1;
    } else{
      toMoveFirstIndex = indexNum[i] +1;
      toMoveLastIndex = indexNum[i+1]-1;
    }
    //Rprintf("First %d, last %d", toMoveFirstIndex,toMoveLastIndex );
    if (toMoveLastIndex < toMoveFirstIndex) {continue;}
    for (int j = toMoveFirstIndex; j<= toMoveLastIndex; j++){
      toMoveFirstPosition = itemIdx[j].second;
      toMoveLastPosition = itemIdx[j+1].second;
      std::vector<BYTE> toMoveRaw(toMoveLastPosition-toMoveFirstPosition);
      fseek(fin, toMoveFirstPosition, SEEK_SET);
      fread((char*)&(toMoveRaw[0]),1, toMoveLastPosition-toMoveFirstPosition, fin);
      fseek(fout, moveToPosition, SEEK_SET);
      fwrite((char*)&(toMoveRaw[0]), 1, toMoveLastPosition-toMoveFirstPosition, fout);
      moveToPosition += toMoveLastPosition-toMoveFirstPosition;
    }
    for (int j = toMoveFirstIndex; j<= toMoveLastIndex; j++)
    {
       itemIdx[j].second -= positionDiff;
       //Rcout << (double)itemIdx[j].second <<"\n";
    }
  }

  itemIdx[lengthOfList].second = moveToPosition;
  fseek(fout, moveToPosition, SEEK_SET);

  //Rcout << itemIdx.size() << "\n";
  int newLengthOfList = lengthOfList - indexNum.size();

  // for (int i = 0; i < lengthOfList; i ++){
  //   Rcout << itemIdx[i].first.c_str() << "\n";
  // }


  // remove elements in the two refference tables.
  std::vector<std::pair<std::string, int64_t> > itemIdxRemain(newLengthOfList);
  int currentDeleteIndex = 0;
  for (int i = 0; i < lengthOfList; i ++)
  {
    if (i == indexNum[currentDeleteIndex])
    {
      currentDeleteIndex++;
      continue;
    }else
    {
      itemIdxRemain[i-currentDeleteIndex].second = itemIdx[i].second;
      itemIdxRemain[i-currentDeleteIndex].first = itemIdx[i].first;
    }
  }

  // for (int i = 0; i < newLengthOfList; i ++){
  //   Rcout << itemIdxRemain[i].first.c_str() << "\n";
  // }
  //

  // write new tables
  writeItemIdx(itemIdxRemain, fout, newLengthOfList);
  fwrite((char *)&(itemIdx[lengthOfList].second), 8, 1, fout);
  std::stable_sort(itemIdxRemain.begin(), itemIdxRemain.end(), cmp);
  writeItemIdx(itemIdxRemain, fout, newLengthOfList);

  int64_t fileLength = ftell(fout);

  //save new length to head
  fseek(fout, 18, SEEK_SET);
  fwrite((char *)&(newLengthOfList), 4, 1, fout);

  fclose(fin);
  fclose(fout);
  cutFile(fileName, fileLength);
  return(ScalarLogical(1)); 
}