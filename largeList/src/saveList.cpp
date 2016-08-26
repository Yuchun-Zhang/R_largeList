#include "largeList.h"

extern "C" SEXP saveList(SEXP object, SEXP file, SEXP append)
{
  //check parameters
  if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("File should be a charater vector of length 1.\n");
  if (TYPEOF(object) != VECSXP) error("Object is not a list.\n");
  //check if the object contains unsupported variable type.
  checkSEXP(object);
  const char *file_name = getFullPath(file);
  //check if the file exists and the format is valid
  if (LOGICAL(append)[0] == true) {checkFile(file_name); checkVersion(file_name);}

  //declare file io pointers.
  FILE *fout;
  FILE *fin;

  //if append is false, truncate the file the write the object
  if (LOGICAL(append)[0] == false) {
    int length_of_list = Rf_xlength(object);
    fout = fopen(file_name, "wb");
    std::vector<std::pair<std::string, int64_t> > pair(length_of_list);
    int64_t last_position;

    //write head, List type and length
    writeVersion(fout);
    std::string list_head("\x13\x00\x00\x00");
    safe_fwrite((char *)list_head.c_str(), 1, 4, fout);
    safe_fwrite((char *) & (length_of_list), 4, 1, fout);

    //array pair stores the positions and names of elements.
    for (int i = 0; i < length_of_list; i++) {
      pair[i].second = ftell(fout);
      writeSEXP(VECTOR_ELT(object, i), fout);
    }
    last_position = ftell(fout);

    //get element names
    int has_name = 0;
    SEXP names_sxp = getObjectName(object, has_name);
    //assign the names to pair
    for (int i = 0; i < length_of_list; i++) {
      std::string name_temp = STRING_ELT(names_sxp, i) == NA_STRING ?
                              std::string(NAMELENGTH, '\xff') :
                              CHAR(STRING_ELT(names_sxp, i));
      pair[i].first = name_temp;
      pair[i].first.resize(NAMELENGTH);
    }

    //write first refference table
    writePair(pair, fout, length_of_list);
    safe_fwrite((char *) & (last_position), 8, 1, fout);

    //sort and write second table
    std::stable_sort(pair.begin(), pair.end(), cmp);
    writePair(pair, fout, length_of_list);

    //write digit indicates if the list has names
    fseek(fout, HAS_NAME_POSITION, SEEK_SET);
    safe_fwrite((char *) & (has_name), 1, 1, fout);

    fclose(fout);
    return (ScalarLogical(1));
  }
  else
  {
    //if append is TRUE, append the new elememts and their positions.
    int length_of_list_append = Rf_length(object);

    //get previous length.
    fin = fopen(file_name, "rb");
    fout = fopen(file_name, "r+b");
    fseek(fin, LENGTH_POSITION, SEEK_SET);
    int length_of_list_old;
    safe_fread((char*)(&length_of_list_old), 4, 1, fin);
    int64_t last_position_old;
    int64_t last_position_new;
    int length_of_list_new = length_of_list_old + length_of_list_append;

    std::vector<std::pair<std::string, int64_t> > pair(length_of_list_new);

    //get previous first table
    fseek(fin, -(8 + NAMELENGTH) * 2 * length_of_list_old - 8, SEEK_END);
    readPair(pair, fin, length_of_list_old);
    safe_fread((char *) & (last_position_old), 8, 1, fin);

    //save elements in the new list
    fseek(fout, last_position_old, SEEK_SET);

    for (int i = 0; i < length_of_list_append; i++) {
      pair[i + length_of_list_old].second = ftell(fout);
      writeSEXP(VECTOR_ELT(object, i), fout);
    }
    last_position_new = ftell(fout);

    // get the new names
    int has_name_new = 0;
    SEXP names_sxp = getObjectName(object, has_name_new);
    for (int i = 0; i < length_of_list_append; i++) {
      std::string name_temp = STRING_ELT(names_sxp, i) == NA_STRING ?
                              std::string(NAMELENGTH, '\xff') :
                              CHAR(STRING_ELT(names_sxp, i));
      pair[i + length_of_list_old].first = name_temp;
      pair[i + length_of_list_old].first.resize(NAMELENGTH);
    }

    //write the first table
    writePair(pair, fout, length_of_list_new);
    safe_fwrite((char *) & (last_position_new), 8, 1, fout);

    //merge to generate the second table and write
    std::stable_sort(pair.begin(), pair.end(), cmp);
    writePair(pair, fout, length_of_list_new);

    //save the new length to file.
    fseek(fout, LENGTH_POSITION, SEEK_SET);
    safe_fwrite((char *) & (length_of_list_new), 4, 1, fout);

    //save digit has_name
    int has_name_old = 0;
    fseek(fin, HAS_NAME_POSITION, SEEK_SET);
    safe_fread((char *) & (has_name_old), 1, 1, fin);
    int has_name_all = (has_name_old || has_name_new);
    fseek(fout, HAS_NAME_POSITION, SEEK_SET);
    safe_fwrite((char *) & (has_name_all), 1, 1, fout);

    fclose(fin);
    fclose(fout);
    return (ScalarLogical(1));
  }
}


