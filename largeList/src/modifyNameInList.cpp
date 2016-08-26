#include "largeList.h"

extern "C" SEXP modifyNameInList(SEXP file, SEXP index, SEXP names)
{
  //check parameters
  if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("File should be a charater vector of length 1.\n");
  if (TYPEOF(names) != STRSXP && TYPEOF(names) != NILSXP) error("Parameter names is neither a character vector nor NULL.\n");
  if (TYPEOF(index) != INTSXP &&  TYPEOF(index) != REALSXP)
    error("Index should be an integer vector or a numeric vector.\n");

  const char *file_name = getFullPath(file);
  //check if the file exists and the format is valid
  checkFile(file_name);
  checkVersion(file_name);

  FILE *fin;
  FILE *fout;
  fin = fopen(file_name, "rb");
  fout = fopen(file_name, "rb+");

  // get the has_name digit 
  int has_name = 0;
  fseek(fin, HAS_NAME_POSITION, SEEK_SET);
  safe_fread((char *) & (has_name), 1, 1, fin);

  int length_of_index = Rf_length(index);
  int length_of_names = Rf_length(names);

  //get list length
  fseek(fin, LENGTH_POSITION, SEEK_SET);
  int length_of_list;
  safe_fread((char*) & (length_of_list), 4, 1, fin);

  //get index
  if (TYPEOF(names) != NILSXP) {
    std::vector<int> index_num;
    index_num.resize(Rf_length(index));
    TYPEOF(index) == INTSXP ?
    index_num.assign(INTEGER(index), INTEGER(index) + Rf_length(index)) :
    index_num.assign(REAL(index), REAL(index) + Rf_length(index));
    for (int i = 0 ; i < Rf_length(index); i++) {
      index_num[i] = index_num[i] - 1;
    }
    int max_index = *std::max_element(index_num.begin(), index_num.end());
    int min_index = *std::min_element(index_num.begin(), index_num.end());
    if (min_index < 0) { fclose(fin); fclose(fout); error("Index should be positive.");}
    if (max_index > length_of_list - 1) { fclose(fin); fclose(fout); error("Index beyonds list length.");}
  }

  //read pair
  fseek(fin, -(8 + NAMELENGTH) * 2 * length_of_list - 8, SEEK_END);
  std::vector<std::pair<std::string, int64_t> > pair(length_of_list + 1);
  readPair(pair, fin, length_of_list);
  safe_fread((char *) & (pair[length_of_list].second), 8, 1, fin);

  // if the list has names and parameter names is NULL, remove the name strings and set has_name to 0.
  if (has_name == 1 && TYPEOF(names) == NILSXP) {
    for (int i = 0; i < length_of_list; i++) {
      pair[i].first = std::string(NAMELENGTH, '\xff');
    }
    has_name = 0;
    fseek(fout, HAS_NAME_POSITION, SEEK_SET);
    safe_fwrite((char *) & (has_name), 1, 1, fout);
  }
  // if the list has no names and parater names is NULL, do nothing.
  if (has_name == 0 && TYPEOF(names) == NILSXP) {}
  // if the list has names and parameter names has some values, modify the names.
  if (has_name == 1 && TYPEOF(names) != NILSXP) {
    for (int i = 0; i < length_of_index; i++) {
      SEXP name_element = STRING_ELT(names, i % length_of_names);
      pair[i].first = name_element == NA_STRING ?
                      std::string(NAMELENGTH, '\xff') :
                      CHAR(name_element);
    }
  }
  // if the list has no names and parameter names has some values, modify them and set has_name to 1.
  if (has_name == 0 && TYPEOF(names) != NILSXP) {
    for (int i = 0; i < length_of_index; i++) {
      SEXP name_element = STRING_ELT(names, i % length_of_names);
      pair[i].first = name_element == NA_STRING ?
                      std::string(NAMELENGTH, '\xff') :
                      CHAR(name_element);
    }
    has_name = 1;
    fseek(fout, HAS_NAME_POSITION, SEEK_SET);
    safe_fwrite((char *) & (has_name), 1, 1, fout);
  }

  //write the pairs back
  fseek(fout, -(8 + NAMELENGTH) * 2 * length_of_list - 8, SEEK_END);
  writePair(pair, fout, length_of_list);
  safe_fwrite((char *) & (pair[length_of_list].second), 8, 1, fout);
  std::stable_sort(pair.begin(), pair.begin() + length_of_list, cmp);
  writePair(pair, fout, length_of_list);

  fclose(fin);
  fclose(fout);
  return (ScalarLogical(1));
}

