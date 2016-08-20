#include "largeList.h"

extern "C" SEXP modifyNameInList(SEXP file, SEXP index, SEXP names)
{
  //check parameters
  if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("File should be a charater vector of length 1.\n");
  if (TYPEOF(names) != STRSXP) error("Parameter names is not a character vector.\n");
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

  int length_of_index = Rf_length(index);
  int length_of_names = Rf_length(names);

  //get list length
  fseek(fin, LENGTH_POSITION, SEEK_SET);
  int length_of_list;
  safe_fread((char*) & (length_of_list), 4, 1, fin);

  //get index
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

  //read pair
  fseek(fin, -(8 + NAMELENGTH) * 2 * length_of_list - 8, SEEK_END);
  std::vector<std::pair<std::string, int64_t> > pair(length_of_list + 1);
  readPair(pair, fin, length_of_list);
  safe_fread((char *) & (pair[length_of_list].second), 8, 1, fin);

  //change names
  for (int i = 0; i < length_of_index; i++) {
    pair[i].first = CHAR(STRING_ELT(names, i % length_of_names));
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

