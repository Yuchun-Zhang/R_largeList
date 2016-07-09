#include "largeList.h"

extern "C" SEXP saveList(SEXP object, SEXP file, SEXP append)
{ 
  //make sure the input object is a list.
  if (TYPEOF(object) != VECSXP){
    throw std::invalid_argument("Object is not a list.");
  }
  
  //declare file io pointers.
  const char *fileName = CHAR(STRING_ELT(file,0));
  std::fstream fout;
  std::fstream fin;
  
  //if append is false or if the file does not exist, save the file in binary as well
  //as the positions of all elements.
  if (LOGICAL(append)[0] == false) {
    int lengthOfList =Rf_xlength(object);
    fout.open(fileName, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
    
    writeVersion(fout);
    BYTE stringHead[5] = {0x13,0x02,0x00,0x00};
    fout.write((char *)&stringHead[0],4);
    fout.write((char *)&(lengthOfList),4);
    
    std::vector<std::pair<std::string, int64_t> > uuidIdx(lengthOfList);
    int64_t lastPosition;
    //Rprintf("Begin to store elements");
    //array headPositions stores the positions of elements.
    for(int i = 0; i < lengthOfList; i++){
      uuidIdx[i].second = fout.tellp();
      writeSEXP(VECTOR_ELT(object, i),fout);
    }
    lastPosition = fout.tellp();
    
    //get element names
    SEXP namesSXP = getObjectName(object);
    
    for(int i = 0; i < lengthOfList; i++){
      std::string nameTemp;
      if (STRING_ELT(namesSXP,i) == NA_STRING){
        nameTemp = "\x00\x00\x00\x00\x00\x00\x00\x00";
      }else{
        nameTemp = CHAR(STRING_ELT(namesSXP, i));
      }
      uuidIdx[i].first = nameTemp;
      uuidIdx[i].first.resize(NAMELENGTH);
    }
    
    // write first refference table
    for(int i = 0; i < lengthOfList; i++){
      fout.write((char *)&(uuidIdx[i].second),8);
      fout.write(uuidIdx[i].first.c_str(),NAMELENGTH);
    }
    fout.write((char *)&(lastPosition),8);

    std::stable_sort(uuidIdx.begin(), uuidIdx.end(), cmp);

    for(int i = 0; i < lengthOfList; i++){
      fout.write((char *)&(uuidIdx[i].second),8);
      fout.write(uuidIdx[i].first.c_str(),NAMELENGTH);
    }
    fout.close();
    return(ScalarLogical(1)); 
  }
  else
  {
    if (FILE *ftest = fopen(fileName, "r")) {
      fclose(ftest);
    }else{
      throw std::invalid_argument("File does not exist!\n");
    }

    //if append is TRUE, append the new elememts and their positions.
    int lengthOfList = Rf_length(object);

    //get previous length.
    fin.open(fileName, std::ios_base::binary | std::ios_base::in);
    fin.seekg(18, std::ios_base::beg);
    int oldLengthOfList;
    fin.read((char*)(&oldLengthOfList), 4);

    fin.seekg(0, std::ios_base::end);

    //get previous positions and name.
    std::vector<std::pair<std::string, int64_t> > uuidIdx(lengthOfList+oldLengthOfList);
    fin.seekg(-(8+NAMELENGTH)*2*oldLengthOfList-8, std::ios_base::end);

    for(int i = 0; i < oldLengthOfList; i++){
      fin.read((char *)&(uuidIdx[i].second),8);
      uuidIdx[i].first.resize(NAMELENGTH);
      fin.read((char *)&((uuidIdx[i].first)[0]),NAMELENGTH);
    }
    int64_t oldLastPosition;
    fin.read((char *)&(oldLastPosition),8);

    //get the second refference table
    std::vector<std::pair<std::string, int64_t> > uuidIdx2Old(oldLengthOfList);
    fin.seekg(-(8+NAMELENGTH)*oldLengthOfList, std::ios_base::end);

    for(int i = 0; i < oldLengthOfList; i++){
      fin.read((char *)&(uuidIdx2Old[i].second),8);
      uuidIdx2Old[i].first.resize(NAMELENGTH);
      fin.read((char *)&((uuidIdx2Old[i].first)[0]),NAMELENGTH);
    }

    //get previous names.
    // int64_t namesWithoutHeadLength = oldFileLength-(8+NAMELENGTH)*2*oldLengthOfList-8 - oldLastPosition -29;
    // std::vector<BYTE> nameWithoutHeadBytes(namesWithoutHeadLength);
    // fin.seekg(oldLastPosition+29, std::ios_base::beg);
    // fin.read((char*)&(nameWithoutHeadBytes[0]),namesWithoutHeadLength);

    fout.open(fileName, std::ios_base::binary | std::ios_base::out | std::ios_base::in);
    //save the new length to file.
    fout.seekp(18, std::ios_base::beg);
    int newLength = oldLengthOfList+lengthOfList;
    fout.write((char *)&(newLength),4);

    //save elements in the new list
    fout.seekp(oldLastPosition, std::ios_base::beg);

    for(int i = 0; i < lengthOfList; i++){
      uuidIdx[i+oldLengthOfList].second = fout.tellp();
      writeSEXP(VECTOR_ELT(object,i),fout);
    }
    int64_t newLastPosition;
    newLastPosition = fout.tellp();

    SEXP namesSXP = getObjectName(object);
    
    // writeNameAttrHead(fout);
    // writeNameAttrLength(newLength,fout);
    // // fout.write((char*)&(nameWithoutHeadBytes[0]),namesWithoutHeadLength-4);
    // writeSTRSXP(namesSXP,fout);
    // writeNameAttrEnd(fout);

    for(int i = 0; i < lengthOfList; i++){
      std::string nameTemp;
      if (STRING_ELT(namesSXP,i) == NA_STRING){
        nameTemp = "\x00\x00\x00\x00\x00\x00\x00\x00";
      }else{
        nameTemp = CHAR(STRING_ELT(namesSXP, i));
      }
      uuidIdx[i+oldLengthOfList].first = nameTemp;
      uuidIdx[i+oldLengthOfList].first.resize(NAMELENGTH);
    }


    // for(int i = 0; i <newLength ; i++){
    //   Rcout << uuidIdx[i].second << "\n";
    // }

    for(int i = 0; i <newLength ; i++){
      fout.write((char *)&(uuidIdx[i].second),8);
      fout.write(uuidIdx[i].first.c_str(),NAMELENGTH);
    }
    fout.write((char *)&(newLastPosition),8);

    std::vector<std::pair<std::string, int64_t> > uuidIdx2New(lengthOfList+oldLengthOfList);
    int currentToInsertIndex = 0;
    int currentOriginIndex = 0;
    int currentNewIndex = 0;
    do {
      if (currentOriginIndex == oldLengthOfList) {
        uuidIdx2New[currentNewIndex] = uuidIdx[oldLengthOfList+currentToInsertIndex];
        currentToInsertIndex ++;
      }else
      if (currentToInsertIndex == lengthOfList)
      {
        uuidIdx2New[currentNewIndex] = uuidIdx2Old[currentOriginIndex];
        currentOriginIndex ++;
      }else
      if (cmp(uuidIdx2Old[currentOriginIndex], uuidIdx[oldLengthOfList+currentToInsertIndex]) == true)
      {
        uuidIdx2New[currentNewIndex] = uuidIdx2Old[currentOriginIndex];
        currentOriginIndex ++;
      }else
      {
        uuidIdx2New[currentNewIndex] = uuidIdx[oldLengthOfList+currentToInsertIndex];
        currentToInsertIndex ++;
      }
      // Rcout << currentToInsertIndex << " "<<currentOriginIndex<< " "<< currentNewIndex << "\n";
      currentNewIndex ++;
    }while( currentNewIndex < lengthOfList+oldLengthOfList);

    for(int i = 0; i < newLength; i++){
      fout.write((char *)&(uuidIdx2New[i].second),8);
      fout.write(uuidIdx2New[i].first.c_str(),NAMELENGTH);
    }

    // std::stable_sort(uuidIdx.begin(), uuidIdx.end(), cmp);
    // for(int i = 0; i < newLength; i++){
    //   fout.write((char *)&(uuidIdx[i].second),8);
    //   fout.write(uuidIdx[i].first.c_str(),NAMELENGTH);
    // }
    fin.close();
    fout.close();
    return(ScalarLogical(1)); 
   }
}


