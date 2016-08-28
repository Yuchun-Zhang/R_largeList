#include "largeList.h"

extern "C" SEXP getListName(SEXP file)
{
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

  //get names
  int has_name = 0;
  fseek(fin, HAS_NAME_POSITION, SEEK_SET);
  safe_fread((char *) & (has_name), 1, 1, fin);
  if (has_name == 0) {
    SEXP names_sxp = PROTECT(R_NilValue);
    UNPROTECT(1);
    fclose(fin);
    return (names_sxp);
  } else {
    SEXP names_sxp = PROTECT(Rf_allocVector(STRSXP, length_of_list));
    std::string name;
    name.resize(NAMELENGTH);
    std::string na_string(NAMELENGTH, '\xff');
    for (int i = 0 ; i < length_of_list; i++) {
      fseek(fin, -( 8 + NAMELENGTH) * length_of_list - (8 + NAMELENGTH) * (length_of_list - i), SEEK_END);
      safe_fread((char *) & (name[0]), 1, NAMELENGTH, fin);
      name == na_string ?
      SET_STRING_ELT(names_sxp, i, NA_STRING) :
      SET_STRING_ELT(names_sxp, i, Rf_mkChar(name.c_str()));
    }
    UNPROTECT(1);
    fclose(fin);
    return (names_sxp);
  }
}
