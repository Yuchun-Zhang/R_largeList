#include "largeList.h"

extern "C" SEXP getListLength(SEXP file){
  //check parameters
  if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("File should be a charater vector of length 1.\n");
  
  const char *fileName = getFullPath(file);
  if(checkFile(fileName) == false) error("File does not exist.\n");
  FILE *fin = fopen(fileName, "rb");
  
  //get length of List
  int lengthOfList;
  fseek(fin, 18, SEEK_SET);
  fread((char*)&(lengthOfList), 4, 1, fin);
  return(ScalarInteger(lengthOfList));
}