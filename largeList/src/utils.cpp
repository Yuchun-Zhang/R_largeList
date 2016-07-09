#include "largeList.h"

bool cmp (std::pair<std::string, int64_t> const & a, std::pair<std::string, int64_t> const & b)
{
  return a.first != b.first?  a.first < b.first : a.second < b.second;
};

void fileBinarySearch (std::fstream &fin, int64_t &position, std::string &name, int &index, int &length){
  int left = 0;
  int right = length-1;
  int mid;
  std::string currentName(8,' ');
  while (left <= right)
  {
    mid = (int) ((left + right) / 2);
    fin.seekg(-16*length+mid*16+8, std::ios_base::end);
    fin.read((char*)&(currentName[0]),8);
    // Rprintf("current name is %d, name is %d, left %d, right %d , cmp %d\n",
    //         currentName.size(),
    //         name.size(),
    //         left,right, currentName > name);
    if (currentName == name)
    {
      index = mid;
      fin.seekg(-16*length+mid*16, std::ios_base::end);
      fin.read((char*)&(position),8);
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

void fileBinarySearchIndex (std::fstream &fin, int64_t &position, int &index, int &length){
  int left = 0;
  int right = length-1;
  int mid;
  int64_t currentPosition;
  while (left <= right)
  {
    mid = (int) ((left + right) / 2);
    fin.seekg(-2*(8+NAMELENGTH)*length-8+mid*(8+NAMELENGTH), std::ios_base::end);
    fin.read((char*)&(currentPosition),8);
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

void writeVersion (std::fstream &fout){
  const int version = 2;
  BYTE format[3] = {'B','\n'};
  fout.write((char *)&format[0],2);
  fout.write((char *)&version,4);
  int R_VERSION_VAR = R_VERSION;
  int R_Version_VAR = R_Version(2,3,0);
  fout.write((char *)&R_VERSION_VAR,4);
  fout.write((char *)&R_Version_VAR,4);
}


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
