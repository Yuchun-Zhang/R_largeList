#include "largeList.h"

extern "C" SEXP getListLength(SEXP file) {
  //check parameters
  if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("File should be a charater vector of length 1.\n");
  const char *file_name = getFullPath(file);
  //check if the file exists and the format is valid
  checkFile(file_name);
  checkVersion(file_name);
  FILE *fin = fopen(file_name, "rb");

  //get length of List
  int length_of_list;
  fseek(fin, LENGTH_POSITION, SEEK_SET);
  safe_fread((char*) & (length_of_list), 4, 1, fin);
  fclose(fin);
  return (ScalarInteger(length_of_list));
}