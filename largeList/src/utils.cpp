#include "largeList.h"

//compare two elements in a pair. 
bool cmp (std::pair<std::string, int64_t> const & a, std::pair<std::string, int64_t> const & b)
{
  return a.first != b.first?  a.first < b.first : a.second < b.second;
};

//given name, find the corresponding position and index.
void fileBinarySearch (FILE *fin, int64_t &position, std::string &name, int &index, int &length){
  int left = 0;
  int right = length-1;
  int mid;
  std::string currentName(NAMELENGTH,' ');
  while (left <= right)
  {
    mid = (int) ((left + right) / 2);
    fseek(fin, -(8+NAMELENGTH)*length+mid*(8+NAMELENGTH)+8, SEEK_END);
    fread((char*)&(currentName[0]), NAMELENGTH , 1, fin);
    // Rprintf("current name is %d, name is %d, left %d, right %d , cmp %d\n",
    //         currentName.size(),
    //         name.size(),
    //         left,right, currentName > name);
    if (currentName == name)
    {
      index = mid;
      fseek(fin, -(8+NAMELENGTH)*length+mid*(8+NAMELENGTH), SEEK_END);
      fread((char*)&(position), 8, 1, fin);
      return;
    }else if (currentName > name)
    {
      right = mid - 1;
    }else{
      left = mid + 1;
    }
  }
  index = -1;
  position = -1;
  return;
}


//given position, find the corresponding index.
void fileBinarySearchIndex (FILE *fin, int64_t &position, int &index, int &length){
  int left = 0;
  int right = length-1;
  int mid;
  int64_t currentPosition;
  while (left <= right)
  {
    mid = (int) ((left + right) / 2);
    fseek(fin, -2*(8+NAMELENGTH)*length-8+mid*(8+NAMELENGTH), SEEK_END);
    fread((char*)&(currentPosition), 8, 1, fin);
    if (currentPosition == position)
    {
      index = mid;
      return;
    }else if (currentPosition > position)
    {
      right = mid - 1;
    }else{
      left = mid + 1;
    }
  }
  return;
}

//output the version info, this part is the same as in saveRDS.
void writeVersion (FILE *fout){
  const int version = 2;
  BYTE format[3] = {'B','\n'};
  fwrite((char *)&format[0], 1, 2, fout);
  fwrite((char *)&version, 1, 4, fout);
  int R_VERSION_VAR = R_VERSION;
  int R_Version_VAR = R_Version(2,3,0);
  fwrite((char *)&R_VERSION_VAR, 1, 4, fout);
  fwrite((char *)&R_Version_VAR, 1, 4, fout);
}

//get the name object of a given R object
SEXP getObjectName(SEXP x){
  SEXP nameSXP = Rf_getAttrib(x,R_NamesSymbol);
  if (nameSXP == R_NilValue){
    nameSXP = PROTECT(Rf_allocVector(STRSXP, Rf_length(x)));
    for (int i = 0 ; i < Rf_length(x); i ++){
      SET_STRING_ELT(nameSXP, i, NA_STRING);
    }
    UNPROTECT(1);
  }
  return(nameSXP);
}

//write the position-name table.
void writeItemIdx(std::vector<std::pair<std::string, int64_t> > &itemIdx, FILE* fout, int &length){
  for(int i = 0; i < length; i++){
    fwrite((char *)&(itemIdx[i].second), 8, 1, fout);
    fwrite(itemIdx[i].first.c_str(), NAMELENGTH, 1, fout);
  }
  return;
}

//read the position-name table.
void readItemIdx(std::vector<std::pair<std::string, int64_t> > &itemIdx, FILE* fin, int &length){
  for(int i = 0; i < length; i++){
    fread((char *)&(itemIdx[i].second),8, 1, fin);
    itemIdx[i].first.resize(NAMELENGTH);
    fread((char *)&((itemIdx[i].first)[0]), NAMELENGTH, 1, fin);
  }
  return;
}

//merge two ordered position-name tables into one
void mergeTwoSortedItemIdx(std::vector<std::pair<std::string, int64_t> > &idx1,
                           std::vector<std::pair<std::string, int64_t> > &idx2,
                           std::vector<std::pair<std::string, int64_t> > &idxNew){
  int idx1Pt = 0;
  int idx2Pt = 0;
  int idxNewPt = 0;
  int idx1Length = idx1.size();
  int idx2Length = idx2.size();
  do {
    if (idx1Pt == idx1Length) {
      idxNew[idxNewPt] = idx2[idx2Pt];
      idx2Pt ++;
    }else
      if (idx2Pt == idx2Length)
      {
        idxNew[idxNewPt] = idx1[idx1Pt];
        idx1Pt ++;
      }else
        if (cmp(idx1[idx1Pt], idx2[idx2Pt]) == true)
        {
          idxNew[idxNewPt] = idx1[idx1Pt];
          idx1Pt ++;
        }else
        {
          idxNew[idxNewPt] = idx2[idx2Pt];
          idx2Pt ++;
        }
        idxNewPt ++;
  }while( idxNewPt < idx1Length+idx2Length);
  return;
}

//check if the file exists or not
bool checkFile(const char *fileName){
  if (FILE *ftest = fopen(fileName, "r")) {
    fclose(ftest);
    return(true);
  }else{
    return(false);
  }
}

//get the full file path
const char* getFullPath(SEXP file){
  const char *fileNameRelative = CHAR(STRING_ELT(file,0));
  const char * res = R_ExpandFileName(fileNameRelative);
  return(res);
}

//cut file to given length, implementation depends on OS.
void cutFile(const char *fileName, const int64_t &fileLength){
#if defined PREDEF_PLATFORM_UNIX
  if (truncate(fileName,fileLength) != 0){
    error("File truncation failed (Unix).");
  }
#endif
  
#if defined PREDEF_PLATFORM_WIN32
  LARGE_INTEGER fileLengthW;
  fileLengthW.QuadPart= fileLength;
  std::wstring fileNameW;
  fileNameW.assign(fileName, fileName + sizeof(fileName) - 1);
  
  HANDLE fh = CreateFileW(fileNameW.c_str(),
                          GENERIC_WRITE, // open for write
                          0,
                          NULL, // default security
                          OPEN_EXISTING, // existing file only
                          FILE_ATTRIBUTE_NORMAL, // normal file
                          NULL);
  SetFilePointerEx(fh, fileLengthW, NULL, 0);
  if (SetEndOfFile(fh) == 0){
    error("File truncation failed (Windows).");
  }
  CloseHandle(fh);
#endif
}