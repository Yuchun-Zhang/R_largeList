#include "largeList.h"
using namespace Rcpp;

int readREALSXP(SEXP & _x, std::fstream &fin, int &length){
  NumericVector x(length);
  fin.read((char*)(&x[0]),8*length);
  _x = x;
  return(true);
}

int readINTSXP(SEXP & _x, std::fstream &fin, int &length){
  IntegerVector x(length);
  fin.read((char*)(&x[0]),4*length);
  _x = x;
  return(true);
}

int readRAWSXP(SEXP & _x, std::fstream &fin, int &length){
  RawVector x(length);
  fin.read((char*)(&x[0]),length);
  _x = x;
  return(true);
}

int readLGLSXP(SEXP & _x, std::fstream &fin, int &length){
  LogicalVector x(length);
  fin.read((char*)(&x[0]),4*length);
  _x = x;
  return(true);
}

int readCHARSXP(SEXP & _x, std::fstream &fin){
  std::vector<BYTE> tempRaw(4);
  std::vector<BYTE> naString(4,0xff);
  fin.read((char *)&(tempRaw[0]),4);
  if (tempRaw == naString){
    _x = NA_STRING;
  }else{
    std::string x;
    int length;
    fin.seekg(-4,std::ios_base::cur);
    fin.read((char *)&(length),4);
    x.resize(length);
    fin.read((char*)(&x[0]),length);
    _x = Rf_mkChar(x.c_str());
  }
  return(true);
}

int readSTRSXP(SEXP & _x, std::fstream &fin, int &length){
  StringVector x(length);
  for (int64_t i = 0; i < length; i++){
    SEXP temp;
    readSEXP(temp, fin);
    x(i) = temp;
  }
  _x = x;
  return(true);
}

int readVECSXP(SEXP & _x, std::fstream &fin, int &length){
  List x(length);
  for (int64_t i = 0; i < length; i++){
    SEXP temp;
    readSEXP(temp, fin);
    x[i] = temp;
  }
  _x = x;
  return(true);
}

int readSYMSXP(SEXP & _x, std::fstream &fin, int &length){
  SEXP chars;
  readSEXP(chars, fin);
  Symbol x(chars);
  _x = x;
  return(true);
}


int readHead(std::fstream &fin, int &type, int &hasAttr, int &hasTag, int &hasObject, int &level){
  int headInfo;
  fin.read((char*)(&headInfo),4);
  type = headInfo & 255;
  hasObject = (headInfo >> 8) & 1;
  hasAttr = (headInfo >> 9) & 1;
  hasTag =  (headInfo >> 10) & 1;
  level = headInfo >> 12; 
  return(true);
}

int readLength(std::fstream &fin, int &length){
  fin.read((char*)(&length),4);
  return(true);
}

int readATTR(SEXP &_x, std::fstream &fin){
  SEXP chan;
  readSEXP(chan, fin);
  SEXP parlist = chan;
  SEXP currentChan = parlist;
  while(true){
    SEXP chan;
    readSEXP(chan, fin);
    if (chan != R_NilValue){
      currentChan = SETCDR(currentChan,chan);
    }else{
      break;
    }
  }
  SET_ATTRIB(_x,parlist);
  return(true);
}


int readSEXP(SEXP & _x, std::fstream &fin)
{
  int type, hasAttr, hasTag, hasObject, level;
  int length;
  readHead(fin, type, hasAttr, hasTag, hasObject, level);
  if (type != NILSXP && type != LISTSXP && type != SYMSXP  && type != CHARSXP){
    readLength(fin,length);
  }
  SEXP element(R_NilValue);
  //Rcout << "TYPE " << type <<"\n";
  //Rcout << "object attr tag"<< hasObject <<" " << hasAttr << " " << hasTag <<"\n";
  switch(type){
  case NILSXP : break;
  case REALSXP  : readREALSXP(element, fin, length); break;
  case INTSXP   : readINTSXP(element, fin, length);  break;
  case STRSXP   : readSTRSXP(element, fin, length);  break;
  case CHARSXP  : readCHARSXP(element, fin);  break;
  case LGLSXP   : readLGLSXP(element, fin, length);  break;
  case VECSXP   : readVECSXP(element, fin, length);   break;
  case RAWSXP   : readRAWSXP(element, fin, length);  break;
  case SYMSXP   : readSYMSXP(element, fin, length);  break;
  case LISTSXP  :  
    {
      SEXP tag;
      SEXP el;
      if (hasTag == 1){ 
        readSEXP(tag,fin);
      }
      readSEXP(el,fin);
      element = Rf_cons(el, R_NilValue); 
      if (hasTag == 1){SET_TAG(element,tag);}
    }break;
  }
  _x = element;
  if (hasAttr == 1){
    readATTR(_x,fin);
  }
  if (hasObject == 1){
    SET_OBJECT(_x,1);
  }
  return(true);
}

// RcppExport SEXP unitTestRead(CharacterVector fileName){
//   std::fstream fin;
//   std::string fname = Rcpp::as<std::string>(fileName);
//   fin.open(fname, std::ios_base::binary | std::ios_base::in);
//   RawVector fileHead(14);
//   fin.read((char*)(&fileHead[0]),14);
//   SEXP element(R_NilValue); 
//   readSEXP(element, fin);
//   fin.close();
//   return(element);
// }