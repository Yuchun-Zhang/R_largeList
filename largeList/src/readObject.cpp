#include "largeList.h"

//read REAL object
inline SEXP readREALSXP(FILE *fin){
  int length;
  readLength(fin,length);
  SEXP x = PROTECT(Rf_allocVector(REALSXP, length));
  fread((char*)(&REAL(x)[0]), 8, length, fin);
  UNPROTECT(1);
  return(x);
}

//read INTEGER object
inline SEXP readINTSXP(FILE *fin){
  int length;
  readLength(fin,length);
  SEXP x = PROTECT(Rf_allocVector(INTSXP, length));
  fread((char*)(&INTEGER(x)[0]), 4, length, fin);
  UNPROTECT(1);
  return(x);
}

//write COMPLEX object
inline SEXP readCPLXSXP(FILE *fin){
  int length;
  readLength(fin,length);
  SEXP x = PROTECT(Rf_allocVector(CPLXSXP, length));
  fread((char*)(&COMPLEX(x)[0]), 16, length, fin);
  UNPROTECT(1);
  return(x);
}

//read RAW object
inline SEXP readRAWSXP(FILE *fin){
  int length;
  readLength(fin,length);
  SEXP x = PROTECT(Rf_allocVector(RAWSXP, length));
  fread((char*)(&RAW(x)[0]), 1, length, fin);
  UNPROTECT(1);
  return(x);
}

//read LOGICAL object
inline SEXP readLGLSXP(FILE *fin){
  int length;
  readLength(fin,length);
  SEXP x = PROTECT(Rf_allocVector(LGLSXP, length));
  fread((char*)(&LOGICAL(x)[0]),4, length, fin);
  UNPROTECT(1);
  return(x);
}

//read CHAR object
inline SEXP readCHARSXP(FILE *fin){
  std::vector<BYTE> tempRaw(4);
  std::vector<BYTE> naString(4,0xff);
  fread((char *)&(tempRaw[0]), 1, 4, fin);
  SEXP _x;
  if (tempRaw == naString){
    _x = PROTECT(NA_STRING);
  }else{
    std::string x;
    int length;
    fseek(fin, -4, SEEK_CUR);
    fread((char *)&(length), 4, 1, fin);
    x.resize(length);
    fread((char*)(&x[0]), 1, length, fin);
    _x = PROTECT(Rf_mkChar(x.c_str()));
  }
  UNPROTECT(1);
  return(_x);
}

//read CHARACTER VECTOR object
inline SEXP readSTRSXP(FILE *fin){
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

//read a LIST object
inline SEXP readVECSXP(FILE *fin){
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

//read a SYM object
inline SEXP readSYMSXP(FILE *fin){
  SEXP chars = PROTECT(readSEXP(fin));
  SEXP x = PROTECT(Rf_install(CHAR(chars)));
  UNPROTECT(2);
  return(x);
}

//read the head info the object
inline void readHead(FILE *fin, int &type, int &hasAttr, int &hasTag, int &hasObject, int &level){
  int headInfo;
  fread((char*)(&headInfo), 4 , 1, fin);
  type = headInfo & 255;
  hasObject = (headInfo >> 8) & 1;
  hasAttr = (headInfo >> 9) & 1;
  hasTag =  (headInfo >> 10) & 1;
  level = headInfo >> 12; 
  return;
}

//read object length
inline void readLength(FILE *fin, int &length){
  fread((char*)(&length), 4, 1, fin);
  return;
}

//read ATTRIBUTE of object
inline void readATTR(SEXP &_x, FILE *fin){
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

//main function of reading a R object
SEXP readSEXP(FILE *fin)
{
  int type, hasAttr, hasTag, hasObject, level;
  readHead(fin, type, hasAttr, hasTag, hasObject, level);
  SEXP element = R_NilValue;
  switch(type){
  case 0xfe : element = PROTECT(R_NilValue); break;
  case REALSXP  : element = PROTECT(readREALSXP(fin)); break;
  case INTSXP   : element = PROTECT(readINTSXP(fin));  break;
  case CPLXSXP  : element = PROTECT(readCPLXSXP(fin));  break;
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
  SETLEVELS(element, level);
  UNPROTECT(1);
  return(element);
}
