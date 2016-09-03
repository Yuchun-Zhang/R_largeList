#include "largeList.h"

extern "C" SEXP modifyInList(SEXP file, SEXP index, SEXP object)
{
  //check parameters
  if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("File should be a charater vector of length 1.\n");
  if (TYPEOF(object) != VECSXP) error("Object is not a list.\n");
  if (index != R_NilValue && TYPEOF(index) != INTSXP &&  TYPEOF(index) != REALSXP && TYPEOF(index) != STRSXP)
    error("Index should be an integer vector, a numeric vector or a character vector.\n");
  try { checkSEXP(object);} catch (int e) { error("Data type not supported. Please check ?largeList");}

  const char *file_name = getFullPath(file);
  //check if the file exists and the format is valid
  checkFile(file_name);
  checkVersion(file_name);

  FILE *fin;
  FILE *fout;
  fin = fopen(file_name, "rb");
  fout = fopen(file_name, "rb+");
  std::vector<int64_t> positions;
  std::vector<int> index_num;
  std::vector<std::string> names;

  //get list length
  fseek(fin, LENGTH_POSITION, SEEK_SET);
  int length_of_list;
  safe_fread((char*) & (length_of_list), 4, 1, fin);

  //get index
  if (TYPEOF(index) == INTSXP ||  TYPEOF(index) == REALSXP) {
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

  if (TYPEOF(index) == STRSXP) {
    names.resize(Rf_length(index));
    for (int i = 0; i < Rf_length(index); i ++) {
      names[i].assign(CHAR(STRING_ELT(index, i)) , Rf_length(STRING_ELT(index, i)));
    }
    positions.resize(names.size());
    index_num.resize(names.size());
    for (int i = 0 ; i < Rf_length(index); i++) {
      names[i].resize(NAMELENGTH);
      fileBinarySearchByName(fin, positions[i], names[i], index_num[i], length_of_list);
      if (positions[i] == -1)
      {
        Rf_warning("Element with name %s not found! \n", names[i].c_str());
        index_num[i] =  -1;
      } else {
        fileBinarySearchByPosition(fin, positions[i], index_num[i], length_of_list);
      }
    }
    //remove invalide elements in the modify list
    int delete_num = 0;
    for (size_t i = 0 ; i < names.size() - delete_num; i++) {
      if (index_num[i] ==  -1) {
        index_num.erase(index_num.begin() + i - delete_num);
        delete_num ++;
      }
    }

    //if no element to modify, exit.
    if (index_num.size() == 0) {
      fclose(fin);
      fclose(fout);
      return (ScalarLogical(1));
    }
  }

  //get all positions and names
  fseek(fin, -(8 + NAMELENGTH) * 2 * length_of_list - 8, SEEK_END);
  std::vector<std::pair<std::string, int64_t> > pair(length_of_list + 1);
  readPair(pair, fin, length_of_list);
  safe_fread((char *) & (pair[length_of_list].second), 8, 1, fin);

  //get new object lenth
  int length_of_object =  Rf_xlength(object);
  std::vector<int64_t> serialized_length_of_object;
  serialized_length_of_object.resize(length_of_object);
  for (int i = 0; i < length_of_object; i ++) {
    int64_t length_of_element = 0;
    lengthOfSEXP(VECTOR_ELT(object, i), length_of_element);
    serialized_length_of_object[i] = length_of_element;
    //Rprintf("Element %d, Serialized Length %lf ", i, (double)length_of_element);
  }

  int length_of_index = index_num.size();
  // move other the elements
  // copy the positions
  std::vector<int64_t> new_positions(length_of_list + 1);
  for (int i = 0; i <= length_of_list; i ++) {
    new_positions[i] = pair[i].second;
    //Rprintf("Element %d, olfPosition %lf ", i, (double)new_positions[i]);
  }
  // change the positions according to the new object lengths.
  for (int i = 0; i < length_of_index; i ++) {
    int64_t diff = serialized_length_of_object[i % length_of_object] - (pair[index_num[i] + 1].second - pair[index_num[i]].second);
    for (int j = index_num[i] + 1; j <= length_of_list; j++) {
      new_positions[j] += diff;
    }
  }

  // for (int i = 0; i<=length_of_list; i ++){
  //   Rprintf("Element %d, new Position %lf ", i, (double)new_positions[i]);
  // }

  // move the date
  int64_t first_move_pos = -1;
  for (int i = 0; i < length_of_list; i ++) {
    //Rprintf("index %d first_move_pos %3.0lf \n",i, (double)first_move_pos);
    if (new_positions[i] == pair[i].second && first_move_pos == -1) continue;
    if (new_positions[i] < pair[i].second) {
      moveData(fin, fout, pair[i].second, pair[i + 1].second, new_positions[i], new_positions[i + 1]);
      //Rprintf("MOVE %3.0ld,%3.0ld,%3.0ld,%3.0ld \n", pair[i].second, pair[i+1].second, new_positions[i], new_positions[i+1]);
    }
    if (new_positions[i] > pair[i].second && first_move_pos == -1) {
      first_move_pos = i;
    }
    if ((new_positions[i + 1] <= pair[i + 1].second || i == length_of_list - 1 ) && first_move_pos != -1) {
      for (int j = i; j >= first_move_pos; j--) {
        moveData(fin, fout, pair[j].second, pair[j + 1].second, new_positions[j], new_positions[j + 1]);
        //Rprintf("MOVE %3.0ld,%3.0ld,%3.0ld,%3.0ld \n", pair[j].second, pair[j+1].second, new_positions[j], new_positions[j+1]);
      }
      first_move_pos = -1;
    }
  }

  // write objects
  for (int i = 0; i <= length_of_list; i ++) {
    pair[i].second = new_positions[i] ;
  }
  for (int i = 0; i < length_of_index; i ++) {
    fseek(fout, pair[index_num[i]].second, SEEK_SET);
    writeSEXP(VECTOR_ELT(object, i % length_of_object), fout);
  }

  fseek(fout, pair[length_of_list].second, SEEK_SET);

  // write new tables
  writePair(pair, fout, length_of_list);
  safe_fwrite((char *) & (pair[length_of_list].second), 8, 1, fout);
  std::stable_sort(pair.begin(), pair.begin() + length_of_list, cmp);
  writePair(pair, fout, length_of_list);

  int64_t file_length = ftell(fout);

  fclose(fin);
  fclose(fout);
  cutFile(file_name, file_length);
  return (ScalarLogical(1));
}