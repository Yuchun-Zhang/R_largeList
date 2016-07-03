#include "largeList.h"
using namespace Rcpp;

int getObjectName(SEXP x, StringVector& nameObject){
  SEXP nameTemp = Rf_getAttrib(x,R_NamesSymbol);
  if (nameTemp != R_NilValue){
    nameObject = nameTemp;
  }
  return(true);
}

int writeNameAttrHead( std::fstream &fout){
  BYTE seg[25] = {0x02,0x04,0x00,0x00,
                0x01,0x00,0x00,0x00,
                0x09,0x00,0x04,0x00,
                0x05,0x00,0x00,0x00,
                0x6e,0x61,0x6d,0x65,0x73,
                0x10,0x00,0x00,0x00};
  fout.write((char *)&(seg[0]),25);
  return(true);
}

int writeNameAttrLength(int &length, std::fstream &fout){
  fout.write((char *)&(length),4);
  return(true);
}

int writeNameAttrEnd( std::fstream &fout){
  BYTE seg[4] = {0xfe,0x00,0x00,0x00};
  fout.write((char *)&(seg[0]),4);
  return(true);
}
