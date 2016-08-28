#include "largeList.h"

extern "C" SEXP removeFromList(SEXP file, SEXP index)
{
  //check parameters
  if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("File should be a charater vector of length 1.\n");
  if (index != R_NilValue && TYPEOF(index) != INTSXP &&  TYPEOF(index) != REALSXP && TYPEOF(index) != STRSXP)
    error("Index should be an integer vector, a numeric vector or a character vector.\n");

  const char *file_name = getFullPath(file);
  //check if the file exists and the format is valid
  checkFile(file_name);
  checkVersion(file_name);

  FILE *fin;
  FILE *fout;
  fin = fopen(file_name, "rb");
  fout = fopen(file_name, "rb+");

  std::vector<int> index_num;
  std::vector<std::pair<std::string, int64_t> > index_pair;

  //get list length
  fseek(fin, LENGTH_POSITION, SEEK_SET);
  int length_of_list;
  safe_fread((char*) & (length_of_list), 4, 1, fin);

  //get index
  int length_of_index = Rf_length(index);
  if (TYPEOF(index) == INTSXP ||  TYPEOF(index) == REALSXP) {
    index_num.resize(length_of_index);
    TYPEOF(index) == INTSXP ?
    index_num.assign(INTEGER(index), INTEGER(index) + length_of_index) :
    index_num.assign(REAL(index), REAL(index) + length_of_index);
    for (int i = 0 ; i < length_of_index; i++) {
      index_num[i] = index_num[i] - 1;
    }
    int max_index = *std::max_element(index_num.begin(), index_num.end());
    int min_index = *std::min_element(index_num.begin(), index_num.end());
    if (min_index < 0) { fclose(fin); fclose(fout); error("Index should be positive.");}
    if (max_index > length_of_list - 1) { fclose(fin); fclose(fout); error("Index beyonds list length.");}
  }

  if (TYPEOF(index) == STRSXP) {
    index_pair.resize(length_of_index);
    for (int i = 0; i < length_of_index; i ++) {
      index_pair[i].first.assign(CHAR(STRING_ELT(index, i)), Rf_length(STRING_ELT(index, i)));
      index_pair[i].first.resize(NAMELENGTH);
    }
    index_num.resize(length_of_index);
    for (int i = 0 ; i < length_of_index; i++) {
      fileBinarySearchByName(fin, index_pair[i].second, index_pair[i].first, index_num[i], length_of_list);
      if (index_pair[i].second == -1) {
        Rf_warning("Element with name %s not found! \n", index_pair[i].first.c_str());
        index_num[i] =  -1;
      } else {
        fileBinarySearchByPosition(fin, index_pair[i].second, index_num[i], length_of_list);
      }
    }
    //remove invalide elements in the delete list
    int delete_num = 0;
    for (int i = 0 ; i < length_of_index; i++) {
      if (index_num[i] ==  -1) {
        index_num.erase(index_num.begin() + i - delete_num);
        delete_num ++;
      }
    }
    //if no element to delete, exit.
    if (index_num.size() == 0) {  
      fclose(fin);
      fclose(fout);
      return (ScalarLogical(1));
    }
    length_of_index = index_num.size();
  }

  //get all positions and names
  std::vector<std::pair<std::string, int64_t> > pair(length_of_list + 1);
  fseek(fin, -(8 + NAMELENGTH) * 2 * length_of_list - 8, SEEK_END);
  readPair(pair, fin, length_of_list);
  safe_fread((char *) & (pair[length_of_list].second), 8, 1, fin);

  //copy new positions
  std::sort(index_num.begin(), index_num.end());
  std::vector<int64_t> new_positions(length_of_list + 1);
  for (int i = 0; i <= length_of_list; i ++) {
    new_positions[i] = pair[i].second;
    // Rprintf("Element %d, olfPosition %lf \n ", i, (double)new_positions[i]);
  }
  // change the positions according to the new value lengths.
  for (int i = 0; i < length_of_index; i ++) {
    for (int j = index_num[i] + 1; j <= length_of_list; j++) {
      new_positions[j] += pair[index_num[i]].second - pair[index_num[i] + 1].second;
    }
  }
  // for (int i = 0; i<=length_of_list; i ++){
  //   Rprintf("Element %d, newPosition %lf \n", i, (double)new_positions[i]);
  // }
  // move the data
  for (int i = 0; i < length_of_list; i ++) {
    if (new_positions[i] < pair[i].second) {
      moveData(fin, fout, pair[i].second, pair[i + 1].second, new_positions[i], new_positions[i + 1]);
      // Rprintf("MOVE %3.0ld,%3.0ld,%3.0ld,%3.0ld \n", pair[i].second, pair[i+1].second, new_positions[i], new_positions[i+1]);
    }
  }

  int new_length_of_list = length_of_list - length_of_index;

  // remove elements in the two refference tables.
  std::vector<std::pair<std::string, int64_t> > pair_remain(new_length_of_list);
  int current_delete_index = 0;
  for (int i = 0; i < length_of_list; i ++) {
    if (i == index_num[current_delete_index]) {
      current_delete_index++;
      continue;
    } else {
      pair_remain[i - current_delete_index].second = new_positions[i];
      pair_remain[i - current_delete_index].first = pair[i].first;
    }
  }

  // write new tables
  fseek(fout, new_positions[length_of_list], SEEK_SET);
  writePair(pair_remain, fout, new_length_of_list);
  safe_fwrite((char *) & (new_positions[length_of_list]), 8, 1, fout);
  std::stable_sort(pair_remain.begin(), pair_remain.end(), cmp);
  writePair(pair_remain, fout, new_length_of_list);

  int64_t file_length = ftell(fout);

  //save new length to head
  fseek(fout, LENGTH_POSITION, SEEK_SET);
  safe_fwrite((char *) & (new_length_of_list), 4, 1, fout);

  fclose(fin);
  fclose(fout);
  cutFile(file_name, file_length);
  return (ScalarLogical(1));
}