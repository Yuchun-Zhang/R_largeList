#include "largeList.h"

extern "C" SEXP getListName(SEXP file)
{
  //check parameters
  if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("File should be a charater vector of length 1.\n");
  const char *fileName = getFullPath(file);
  if(checkFile(fileName) == false) error("File does not exist.\n");
  FILE *fin = fopen(fileName, "rb");
      
  //get length of List
  int lengthOfList;
  fseek(fin, 18, SEEK_SET);
  fread((char*)&(lengthOfList), 4, 1, fin);

  //get names
  SEXP namesSXP = PROTECT(Rf_allocVector(STRSXP, lengthOfList));
  char name[NAMELENGTH];
  for(int i = 0 ; i < lengthOfList; i++){
    fseek(fin,-(8+NAMELENGTH)*lengthOfList -(8+NAMELENGTH)*(lengthOfList-i), SEEK_END);
    fread(&(name[0]), 1, NAMELENGTH, fin);
    SET_STRING_ELT(namesSXP, i, Rf_mkChar(name));
  }
  UNPROTECT(1);
  return(namesSXP);
}
