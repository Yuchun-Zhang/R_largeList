#include "largeList.h"

extern "C" SEXP readList(SEXP file, SEXP index = R_NilValue)
{
  //check parameters
  if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("File should be a charater vector of length 1.");
  if (index != R_NilValue && TYPEOF(index) != INTSXP &&  TYPEOF(index) != REALSXP && TYPEOF(index) != STRSXP)
    error("Index should be a NULL, an integer vector, a numeric vector or a character vector.");

  const char *file_name = getFullPath(file);
  //check if the file exists and the format is valid
  checkFile(file_name);
  checkVersion(file_name);
  FILE *fin = fopen(file_name, "rb");

  //get file length
  fseek(fin, 0, SEEK_END);
  int64_t file_size = ftell(fin);
  std::vector<std::pair<std::string, int64_t> > index_pair;

  //get list length
  fseek(fin, LENGTH_POSITION, SEEK_SET);
  int length_of_list;
  safe_fread((char*) & (length_of_list), 4, 1, fin);
  std::vector<int> index_num;

  int length_of_index = 0;
  //if index is not given.
  if (index == R_NilValue) {
    index_num.resize(length_of_list);
    for (int i = 0 ; i < length_of_list; i++) {
      index_num[i] = i;
    }
    index_pair.resize(length_of_list);
    fseek(fin, -(8 + NAMELENGTH) * 2 * length_of_list - 8, SEEK_END);
    readPair(index_pair, fin, length_of_list);
    length_of_index = length_of_list;
  }


  //check if given index is numeric.
  if (TYPEOF(index) == INTSXP ||  TYPEOF(index) == REALSXP) {
    length_of_index = Rf_length(index);
    index_num.resize(length_of_index);
    TYPEOF(index) == INTSXP ?
    index_num.assign(INTEGER(index), INTEGER(index) + length_of_index) :
    index_num.assign(REAL(index), REAL(index) + length_of_index);
    for (int i = 0 ; i < length_of_index; i++) {
      index_num[i] = index_num[i] - 1;
    }
    //check the range of indicies
    int max_index = *std::max_element(index_num.begin(), index_num.end());
    int min_index = *std::min_element(index_num.begin(), index_num.end());
    if (min_index < 0) { fclose(fin); error("Index should be positive."); }
    if (max_index > length_of_list - 1) { fclose(fin); error("Index beyonds list length.");}

    index_pair.resize(length_of_list);
    for (int i = 0 ; i < length_of_index; i++) {
      fseek(fin, -2 * (8 + NAMELENGTH) * length_of_list - 8 + index_num[i] * (8 + NAMELENGTH), SEEK_END);
      safe_fread((char*) & (index_pair[i].second), 8, 1, fin);
      index_pair[i].first.resize(NAMELENGTH);
      safe_fread((char*)&index_pair[i].first[0], NAMELENGTH, 1, fin);
    }
  }

  if (TYPEOF(index) == STRSXP) {
    length_of_index = Rf_length(index);
    index_pair.resize(length_of_index);
    for (int i = 0; i < length_of_index; i ++) {
      index_pair[i].first = charsxpToString(STRING_ELT(index, i));   
    }
    index_num.resize(length_of_index);
    for (int i = 0 ; i < length_of_index; i++) {
      if (STRING_ELT(index, i) == NA_STRING) {
        index_pair[i].second = -1;
      } else {
        fileBinarySearchByName(fin, index_pair[i].second, index_pair[i].first, index_num[i], length_of_list);
      }
    }
  }

  //get elements.
  SEXP output_list = PROTECT(Rf_allocVector(VECSXP, length_of_index));
  for (int i = 0; i < length_of_index; i++ ) {
    if (index_pair[i].second == -1) {
      //Rf_warning("Element %s not found! \n", index_pair[i].first.c_str());
      SET_VECTOR_ELT(output_list, i, R_NilValue);
    } else {
      if (index_pair[i].second > file_size ) {
        Rprintf("index %d ,headPosition %lf, file_size %lf", i, (double)index_pair[i].second, (double)file_size);
        fclose(fin);
        error("Head position exceeds file length. Maybe the file is not generated by saveLargeList function.");
      }
      fseek(fin, index_pair[i].second, SEEK_SET);
      SEXP element = PROTECT(readSEXP(fin));
      SET_VECTOR_ELT(output_list, i, element);
      UNPROTECT_PTR(element);
    }
  }

  //get names
  int has_name = 0;
  fseek(fin, HAS_NAME_POSITION, SEEK_SET);
  safe_fread((char *) & (has_name), 1, 1, fin);
  if (has_name == 1) {
    SEXP names_sxp = PROTECT(Rf_allocVector(STRSXP, length_of_index));
    std::string na_string(NAMELENGTH, '\xff');
    for (int i = 0; i < length_of_index; i++ ) {
      index_pair[i].first == na_string ?
      SET_STRING_ELT(names_sxp, i, NA_STRING) :
      SET_STRING_ELT(names_sxp, i, Rf_mkChar(index_pair[i].first.c_str()));
    }
    //set names
    Rf_setAttrib(output_list, R_NamesSymbol, names_sxp);
    UNPROTECT(1);
  }
  fclose(fin);
  UNPROTECT(1);
  return (output_list);
}