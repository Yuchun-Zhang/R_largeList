#include "largeList.h"

extern "C" SEXP getListLength(SEXP file){
  const char *fileName = CHAR(STRING_ELT(file,0));
  std::fstream fin;
  fin.open(fileName, std::ios_base::binary | std::ios_base::in);
  
  //get length of List
  fin.seekg(18, std::ios_base::beg);
  int lengthOfList;
  fin.read((char*)&(lengthOfList), 4);
  return(ScalarInteger(lengthOfList));
}