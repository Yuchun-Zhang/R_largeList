#include "largeList.h"

extern "C" SEXP removeFromList(SEXP file, SEXP index)
{
  const char *fileName = CHAR(STRING_ELT(file,0));
  std::fstream fout;
  std::fstream fin;
  fout.open(fileName, std::ios_base::binary | std::ios_base::out | std::ios_base::in);
  fin.open(fileName, std::ios_base::binary | std::ios_base::in);
  std::vector<int64_t> positions;
  std::vector<int> indexNum;
  std::vector<std::string> names;
  
  //get list length
  fin.seekg(18, std::ios_base::beg);
  int lengthOfList;
  fin.read((char*)&(lengthOfList), 4);
  
  if (TYPEOF(index) == INTSXP ||  TYPEOF(index) == REALSXP){
    indexNum.resize(Rf_length(index));
    if (TYPEOF(index) == INTSXP){
      indexNum.assign(INTEGER(index),INTEGER(index)+Rf_length(index));
    }else{
      indexNum.assign(REAL(index),REAL(index)+Rf_length(index));
    }
    for(int i = 0 ; i < Rf_length(index); i++){
      indexNum[i] = indexNum[i]-1;
    }
    int maxIndex = *std::max_element(indexNum.begin(), indexNum.end());
    int minIndex = *std::min_element(indexNum.begin(), indexNum.end());
    if (minIndex < 0){
      throw std::range_error("Invalid index: index should be positive");
    }
    if (maxIndex > lengthOfList-1){
      throw std::range_error("Invalid index: index beyonds list length");
    }
  }
  
  if (TYPEOF(index) == STRSXP){
    names.resize(Rf_length(index));
    for (int i = 0; i < Rf_length(index); i ++){
      names[i].assign(CHAR(STRING_ELT(index,i)),Rf_length(STRING_ELT(index,i)));
    }
    positions.resize(names.size());
    indexNum.resize(names.size());
    for(int i = 0 ; i < Rf_length(index); i++){
      names[i].resize(8);
      fileBinarySearch(fin,positions[i],names[i],indexNum[i],lengthOfList);
      if (positions[i] == -1)
      {
        Rf_warning("Element with name %s not found! \n",names[i].c_str());
        indexNum[i] =  -1;
      }else{
        fileBinarySearchIndex(fin,positions[i],indexNum[i],lengthOfList);
      }
      // Rcout << "table1 " << indexNum[i] <<"\n";
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
  fin.seekg(-(8+NAMELENGTH)*2*lengthOfList-8, std::ios_base::end);
  std::vector<std::pair<std::string, int64_t> > uuidIdx(lengthOfList+1);
  for (int i = 0; i < lengthOfList; i ++){
    fin.read((char *)&(uuidIdx[i].second),8);
    uuidIdx[i].first.resize(8);
    fin.read((char *)&((uuidIdx[i].first)[0]),NAMELENGTH);
  }
  fin.read((char *)&(uuidIdx[lengthOfList].second),8);
  
  moveToPosition = uuidIdx[indexNum[0]].second;
  // Rcout << indexNum[0] << "\n";
  
  //move elements
  for (size_t i = 0; i < indexNum.size(); i ++){
     // Rcout << indexNum[i] <<" "<< lengthOfList <<" "<< i << " "<< indexNum.size() << "\n";
    if (indexNum[i] == lengthOfList -1){break;} // if the to remove element is the last in the list, do nothing.
    positionDiff += uuidIdx[indexNum[i]+1].second - uuidIdx[indexNum[i]].second;
    if (i == indexNum.size() -1){ 
      toMoveFirstIndex = indexNum[i] +1;
      toMoveLastIndex = lengthOfList -1;
    } else{
      toMoveFirstIndex = indexNum[i] +1;
      toMoveLastIndex = indexNum[i+1]-1;
    }
    // Rcout << toMoveFirstIndex << " "<<toMoveLastIndex<<"\n";
    if (toMoveLastIndex < toMoveFirstIndex) {continue;}
    for (int j = toMoveFirstIndex; j<= toMoveLastIndex; j++){
      toMoveFirstPosition = uuidIdx[j].second;
      toMoveLastPosition = uuidIdx[j+1].second;
      std::vector<BYTE> toMoveRaw(toMoveLastPosition-toMoveFirstPosition);
      fin.seekg(toMoveFirstPosition, std::ios_base::beg);
      fin.read((char*)&(toMoveRaw[0]),toMoveLastPosition-toMoveFirstPosition);
      fout.seekp(moveToPosition,std::ios_base::beg);
      fout.write((char*)&(toMoveRaw[0]),toMoveLastPosition-toMoveFirstPosition);
      moveToPosition += toMoveLastPosition-toMoveFirstPosition;
    }
    for (int j = toMoveFirstIndex; j<= toMoveLastIndex; j++)
    {
       uuidIdx[j].second -= positionDiff;
       //Rcout << (double)uuidIdx[j].second <<"\n";
    }
  }
  
  uuidIdx[lengthOfList].second = moveToPosition;
  fout.seekp(moveToPosition, std::ios_base::beg);
  
  //Rcout << uuidIdx.size() << "\n";
  int newLengthOfList = lengthOfList - indexNum.size();

  // for (int i = 0; i < lengthOfList; i ++){
  //   Rcout << uuidIdx[i].first.c_str() << "\n";
  // }
  
  
  // remove elements in the two refference tables.
  std::vector<std::pair<std::string, int64_t> > uuidIdxRemain(newLengthOfList);
  int currentDeleteIndex = 0;
  for (int i = 0; i < lengthOfList; i ++)
  {
    if (i == indexNum[currentDeleteIndex])
    {
      currentDeleteIndex++;
      continue;
    }else
    {
      uuidIdxRemain[i-currentDeleteIndex].second = uuidIdx[i].second;
      uuidIdxRemain[i-currentDeleteIndex].first = uuidIdx[i].first;
    }
  }
  
  // for (int i = 0; i < newLengthOfList; i ++){
  //   Rcout << uuidIdxRemain[i].first.c_str() << "\n";
  // }
  // 
  
  // write new tables
  for(int i = 0; i <newLengthOfList ; i++){
    fout.write((char *)&(uuidIdxRemain[i].second),8);
    fout.write(uuidIdxRemain[i].first.c_str(),NAMELENGTH);
  }
  fout.write((char *)&(uuidIdx[lengthOfList].second),8);

  
  std::stable_sort(uuidIdxRemain.begin(), uuidIdxRemain.end(), cmp);

  for(int i = 0; i < newLengthOfList; i++){
    fout.write((char *)&(uuidIdxRemain[i].second),8);
    fout.write(uuidIdxRemain[i].first.c_str(),NAMELENGTH);
  }
  
  int64_t fileLength = fout.tellp();
  
  //save new length to head
  fout.seekp(18, std::ios_base::beg);
  fout.write((char *)&(newLengthOfList),4);
  
  fin.close();
  fout.close();
#if defined PREDEF_PLATFORM_UNIX
  //Rcout << "unix\n"; 
  if (truncate(fileName,fileLength) != 0){
    Rprintf("File truncation failed (Unix)!");
  }
#endif
  
#if defined PREDEF_PLATFORM_WIN32
  //Rcout << "windows\n"; 
  LARGE_INTEGER fileLengthW;
  fileLengthW.QuadPart= fileLength;
  std::wstring fileNameW;
  fileNameW.assign(fileName, fileName + sizeof(fileName) - 1);
  
  HANDLE fh = CreateFileW(fileNameW.c_str(), 
                     GENERIC_WRITE, // open for write
                     0,
                     NULL, // default security
                     OPEN_EXISTING, // existing file only
                     FILE_ATTRIBUTE_NORMAL, // normal file
                     NULL);
  SetFilePointerEx(fh, fileLengthW, NULL, 0);
  if (SetEndOfFile(fh) == 0){
    DWORD lastError = GetLastError();
    Rprintf("File truncation failed (Windows)!");
  }
  CloseHandle(fh);
#endif

  return(ScalarLogical(1)); 
}