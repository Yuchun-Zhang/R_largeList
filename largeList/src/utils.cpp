#include "largeList.h"

//compare two elements in a pair.
bool cmp (std::pair<std::string, int64_t> const & a, std::pair<std::string, int64_t> const & b)
{
  return a.first != b.first ?  a.first < b.first : a.second < b.second;
};

//given name, find the corresponding position and index.
void fileBinarySearchByName (FILE *fin, int64_t &position, std::string &name, int &index, int &length) {
  int64_t left = 0;
  int64_t right = length - 1;
  int64_t mid;
  std::string current_name(NAMELENGTH, '\xff');
  while (left <= right) {
    mid =  (left + right) / 2;
    fseek(fin, -(8 + NAMELENGTH) * length + mid * (8 + NAMELENGTH) + 8, SEEK_END);
    safe_fread((char*) & (current_name[0]), NAMELENGTH , 1, fin);
    //Rprintf("%d \n", current_name.size());
    //Rprintf("%d \n", name.size());
    //Rprintf("%04x \n", current_name[0]);
    //Rprintf("%04x \n", name[0]);
    if (current_name == name) {
      index = mid;
      fseek(fin, -(8 + NAMELENGTH) * length + mid * (8 + NAMELENGTH), SEEK_END);
      safe_fread((char*) & (position), 8, 1, fin);
      return;
    } else if (current_name > name) {
      right = mid - 1;
    } else {
      left = mid + 1;
    }
  }
  index = -1;
  position = -1;
  return;
}


//given position, find the corresponding index.
void fileBinarySearchByPosition (FILE *fin, int64_t &position, int &index, int &length) {
  int64_t left = 0;
  int64_t right = length - 1;
  int64_t mid;
  int64_t current_position;
  while (left <= right) {
    mid = (left + right) / 2;
    fseek(fin, -2 * (8 + NAMELENGTH) * length - 8 + mid * (8 + NAMELENGTH), SEEK_END);
    safe_fread((char*) & (current_position), 8, 1, fin);
    if (current_position == position) {
      index = mid;
      return;
    } else if (current_position > position) {
      right = mid - 1;
    } else {
      left = mid + 1;
    }
  }
  return;
}

//output the version info, this part is the same as in saveRDS.
void writeVersion (FILE *fout) {
  std::string head("LARGELIST ");
  safe_fwrite((char*)head.c_str(), 1, 10, fout);
  int current_version = CURRENT_VERSION;
  int readable_version = READABLE_VERSION;
  safe_fwrite((char *)&current_version, 1, 4, fout);
  safe_fwrite((char *)&readable_version, 1, 4, fout);
  int has_name = 0;
  safe_fwrite((char *)&has_name, 1, 1, fout);
  std::string reserved_string(7, '\x00');
  safe_fwrite((char*)reserved_string.c_str(), 1, 7, fout);
}

void checkVersion(const char *file_name) {
  FILE *fin = fopen(file_name, "r");
  fseek(fin, 0, SEEK_END);
  int64_t length_of_file = ftell(fin);
  if (length_of_file < 26) {
    error("unkown file format!");
  }
  std::string right_head("LARGELIST ");
  std::string head(10, '\x00');
  fseek(fin, 0, SEEK_SET);
  safe_fread((char *)&head[0], 1, 10, fin);
  if (right_head.compare(head) != 0) {
    error("unkown file format!");
  }
  fseek(fin, 14, SEEK_SET);
  int readable_version;
  safe_fread((char *)&readable_version, 4, 1, fin);
  if (readable_version < READABLE_VERSION) {
    fclose(fin);
    error("can only read files created by version above %d.%d.%d",
          (READABLE_VERSION >> 8) & 0x0f,
          (READABLE_VERSION >> 4) & 0x0f,
          (READABLE_VERSION >> 0) & 0x0f);
  }
  fclose(fin);
  return;
}

//get the name object of a given R object
SEXP getObjectName(SEXP x, int & has_name) {
  has_name = 1;
  SEXP name_sxp = Rf_getAttrib(x, R_NamesSymbol);
  if (name_sxp == R_NilValue) {
  	has_name = 0;
    name_sxp = PROTECT(Rf_allocVector(STRSXP, Rf_length(x)));
    for (int i = 0 ; i < Rf_length(x); i ++) {
      SET_STRING_ELT(name_sxp, i, NA_STRING);
    }
    UNPROTECT(1);
  }
  return name_sxp;
}

//write the position-name table.
void writePair(std::vector<std::pair<std::string, int64_t> > &pair, FILE* fout, int &length) {
  for (int i = 0; i < length; i++) {
    safe_fwrite((char *) & (pair[i].second), 8, 1, fout);
    safe_fwrite((char*)pair[i].first.c_str(), NAMELENGTH, 1, fout);
  }
  return;
}

//read the position-name table.
void readPair(std::vector<std::pair<std::string, int64_t> > &pair, FILE* fin, int &length) {
  for (int i = 0; i < length; i++) {
    safe_fread((char *) & (pair[i].second), 8, 1, fin);
    pair[i].first.resize(NAMELENGTH);
    safe_fread((char *) & ((pair[i].first)[0]), NAMELENGTH, 1, fin);
  }
  return;
}

//merge two ordered position-name tables into one
void mergeTwoSortedPair(std::vector<std::pair<std::string, int64_t> > &pair1,
                        std::vector<std::pair<std::string, int64_t> > &pair2,
                        std::vector<std::pair<std::string, int64_t> > &pair_new) {
  int pair1_pt = 0;
  int pair2_pt = 0;
  int pair_new_pt = 0;
  int pair1_length = pair1.size();
  int pair2_length = pair2.size();
  do {
    if (pair1_pt == pair1_length) {
      pair_new[pair_new_pt] = pair2[pair2_pt];
      pair2_pt ++;
    } else if (pair2_pt == pair2_length)
    {
      pair_new[pair_new_pt] = pair1[pair1_pt];
      pair1_pt ++;
    } else if (cmp(pair1[pair1_pt], pair2[pair2_pt]) == true)
    {
      pair_new[pair_new_pt] = pair1[pair1_pt];
      pair1_pt ++;
    } else
    {
      pair_new[pair_new_pt] = pair2[pair2_pt];
      pair2_pt ++;
    }
    pair_new_pt ++;
  } while ( pair_new_pt < pair1_length + pair2_length);
  return;
}

//check if the file exists or not
void checkFile(const char *file_name) {
  if (FILE *ftest = fopen(file_name, "r")) {
    fclose(ftest);
    return;
  } else {
    fclose(ftest);
    error("File does not exist.");
  }
}

//get the full file path
const char* getFullPath(SEXP file) {
  const char *file_name_relative = CHAR(STRING_ELT(file, 0));
  const char * res = R_ExpandFileName(file_name_relative);
  return res;
}


//cut file to given length, implementation depends on OS.
void cutFile(const char *file_name, const int64_t &file_length) {
#if defined PREDEF_PLATFORM_UNIX
  if (truncate(file_name, file_length) != 0) {
    error("File truncation failed (Unix).");
  }
#endif

#if defined PREDEF_PLATFORM_WIN32
  LARGE_INTEGER file_length_w;
  file_length_w.QuadPart = file_length;
  int retries = 0;
  HANDLE fh;
  do {
    fh = CreateFile((LPCTSTR)file_name,
                    GENERIC_WRITE, // open for write
                    0,
                    NULL, // default security
                    OPEN_EXISTING, // existing file only
                    FILE_ATTRIBUTE_NORMAL, // normal file
                    NULL);
    if (fh == INVALID_HANDLE_VALUE) {
      DWORD last_error = GetLastError();
      if (ERROR_SHARING_VIOLATION == last_error) {
        retries += 1;
        Sleep(RETRYDELAY);
        continue;
      } else {
        error("File truncation failed (Windows), get file handle error. Error Code %d .", last_error);
      }
    } 
    break;
  } while (retries < MAXRETRIES);
  if (fh == INVALID_HANDLE_VALUE) {
    error("Tried to trauncate file but it was already in use");
  }

  SetFilePointerEx(fh, file_length_w, NULL, 0);
  if (SetEndOfFile(fh) == 0) {
    DWORD last_error = GetLastError();
    error("File truncation failed (Windows), Error Code %d .", last_error);
  }
  CloseHandle(fh);
#endif
}

// move data in the file
void moveData(FILE *fin, FILE*fout, const int64_t &move_from_start_pos, const int64_t &move_from_end_pos,
              const int64_t &move_to_start_pos, const int64_t &move_to_end_pos) {
  if (move_from_end_pos - move_from_start_pos != move_to_end_pos - move_to_start_pos) {return;}
  std::vector<BYTE> to_move_raw(move_from_end_pos - move_from_start_pos);
  fseek(fin, move_from_start_pos, SEEK_SET);
  safe_fread((char*) & (to_move_raw[0]), 1, move_from_end_pos - move_from_start_pos, fin);
  fseek(fout, move_to_start_pos, SEEK_SET);
  safe_fwrite((char*) & (to_move_raw[0]), 1, move_to_end_pos - move_to_start_pos, fout);
  return;
}

//safe fwrite
void safe_fwrite(char *data, int nbytes, int nblocks, FILE *fout) {
  int64_t initial_ptr_position = ftell(fout);
  int retries = 0;
  while (((int)fwrite(data, nbytes, nblocks, fout) != nblocks) && (retries < MAXRETRIES)) {
    fseek(fout, initial_ptr_position, SEEK_SET);
    retries ++;
  }
  if (retries == MAXRETRIES) {
    error("fwrite failed!");
  }
  return;
}


//safe fread
void safe_fread(char *data, int nbytes, int nblocks, FILE *fin) {
  int64_t initial_ptr_position = ftell(fin);
  int retries = 0;
  while (((int)fread(data, nbytes, nblocks, fin) != nblocks) && (retries < MAXRETRIES)) {
    fseek(fin, initial_ptr_position, SEEK_SET);
    retries ++;
  }
  if (retries == MAXRETRIES) {
    error("fread failed!");
  }
  return;
}

//check file and version (external)
extern "C" SEXP checkFileAndVersionExternal(SEXP file)
{
  if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("File should be a charater vector of length 1.\n");
  const char *file_name = getFullPath(file);
  checkFile(file_name);
  checkVersion(file_name);
  return (ScalarLogical(1));
}

//replace all character ch1 in string str with ch2.
void replaceChar(std::string &str, char ch1, char ch2) {
  for (int i = 0; i < str.size(); i++ ) {
    if (str[i] == ch1) { str[i] = ch2; }
  }
  return;
}

//
std::string charsxpToString(SEXP char_sexp) {
  std::string str;
  if (char_sexp== NA_STRING) {
    str = std::string(NAMELENGTH, '\xff');
  } else {
    str = std::string(NAMELENGTH, '\x00');
    str.replace(0, Rf_length(char_sexp), CHAR(char_sexp));
  }
  return(str);
}