#include "largeList.h"

SEXP readREALSXP(std::fstream &fin){
  int length;
  readLength(fin,length);
  SEXP x = PROTECT(Rf_allocVector(REALSXP, length));
  fin.read((char*)(&REAL(x)[0]),8*length);
  UNPROTECT(1);
  return(x);
}

SEXP readINTSXP(std::fstream &fin){
  int length;
  readLength(fin,length);
  SEXP x = PROTECT(Rf_allocVector(INTSXP, length));
  fin.read((char*)(&INTEGER(x)[0]),4*length);
  UNPROTECT(1);
  return(x);
}

SEXP readRAWSXP(std::fstream &fin){
  int length;
  readLength(fin,length);
  SEXP x = PROTECT(Rf_allocVector(RAWSXP, length));
  fin.read((char*)(&RAW(x)[0]),length);
  UNPROTECT(1);
  return(x);
}

SEXP readLGLSXP(std::fstream &fin){
  int length;
  readLength(fin,length);
  SEXP x = PROTECT(Rf_allocVector(LGLSXP, length));
  fin.read((char*)(&LOGICAL(x)[0]),4*length);
  UNPROTECT(1);
  return(x);
}

SEXP readCHARSXP(std::fstream &fin){
  std::vector<BYTE> tempRaw(4);
  std::vector<BYTE> naString(4,0xff);
  fin.read((char *)&(tempRaw[0]),4);
  SEXP _x;
  if (tempRaw == naString){
    _x = PROTECT(NA_STRING);
  }else{
    std::string x;
    int length;
    fin.seekg(-4,std::ios_base::cur);
    fin.read((char *)&(length),4);
    x.resize(length);
    fin.read((char*)(&x[0]),length);
    _x = PROTECT(Rf_mkChar(x.c_str()));
  }
  UNPROTECT(1);
  return(_x);
}

SEXP readSTRSXP(std::fstream &fin){
  int length;
  readLength(fin,length);
  SEXP x = PROTECT(Rf_allocVector(STRSXP, length));
  for (int64_t i = 0; i < length; i++){
    SEXP temp = PROTECT(readSEXP(fin));
    SET_STRING_ELT(x, i, temp);
    UNPROTECT(1);
  }
  UNPROTECT(1);
  return(x);
}

SEXP readVECSXP(std::fstream &fin){
  int length;
  readLength(fin,length);
  SEXP x = PROTECT(Rf_allocVector(VECSXP, length));
  for (int64_t i = 0; i < length; i++){
    SEXP temp = PROTECT(readSEXP(fin));
    SET_VECTOR_ELT(x,i,temp);
    UNPROTECT(1);
  }
  UNPROTECT(1);
  return(x);
}

SEXP readSYMSXP(std::fstream &fin){
  SEXP chars = PROTECT(readSEXP(fin));
  SEXP x = PROTECT(Rf_install(CHAR(chars)));
  UNPROTECT(2);
  return(x);
}


void readHead(std::fstream &fin, int &type, int &hasAttr, int &hasTag, int &hasObject, int &level){
  int headInfo;
  fin.read((char*)(&headInfo),4);
  type = headInfo & 255;
  hasObject = (headInfo >> 8) & 1;
  hasAttr = (headInfo >> 9) & 1;
  hasTag =  (headInfo >> 10) & 1;
  level = headInfo >> 12; 
  return;
}

void readLength(std::fstream &fin, int &length){
  fin.read((char*)(&length),4);
  return;
}

void readATTR(SEXP &_x, std::fstream &fin){
  SEXP parlist = PROTECT(readSEXP(fin));
  SEXP currentChan = parlist;
  while(true){
    SEXP chan = PROTECT(readSEXP(fin));
    if (chan != R_NilValue){
      currentChan = SETCDR(currentChan,chan);
      UNPROTECT_PTR(chan);
    }else{
      UNPROTECT_PTR(chan);
      break;
    }
  }
  SET_ATTRIB(_x,parlist);
  UNPROTECT_PTR(parlist);
  return;
}


SEXP readSEXP(std::fstream &fin)
{
  int type, hasAttr, hasTag, hasObject, level;
  readHead(fin, type, hasAttr, hasTag, hasObject, level);
  //Rcout << "TYPE " << type <<"\n";
  //Rcout << "object attr tag"<< hasObject <<" " << hasAttr << " " << hasTag <<"\n";
  SEXP element;
  switch(type){
  case 0xfe : element = PROTECT(R_NilValue); break;
  case REALSXP  : element = PROTECT(readREALSXP(fin)); break;
  case INTSXP   : element = PROTECT(readINTSXP(fin));  break;
  case STRSXP   : element = PROTECT(readSTRSXP(fin));  break;
  case CHARSXP  : element = PROTECT(readCHARSXP(fin)); break;
  case LGLSXP   : element = PROTECT(readLGLSXP(fin));  break;
  case VECSXP   : element = PROTECT(readVECSXP(fin));   break;
  case RAWSXP   : element = PROTECT(readRAWSXP(fin));  break;
  case SYMSXP   : element = PROTECT(readSYMSXP(fin));  break;
  case LISTSXP  :  
    {
      SEXP tag;
      SEXP el;
      if (hasTag == 1){ 
        tag = PROTECT(readSEXP(fin));
      }
      el = PROTECT(readSEXP(fin));
      element = PROTECT(Rf_cons(el, R_NilValue));
      UNPROTECT_PTR(el);
      if (hasTag == 1){SET_TAG(element,tag); UNPROTECT_PTR(tag);}
    }break;
  }
  if (hasAttr == 1){
    readATTR(element,fin);
  }
  if (hasObject == 1){
    SET_OBJECT(element,1);
  }
  UNPROTECT_PTR(element);
  return(element);
}
