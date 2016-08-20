#include "largeList.h"

//read REAL object
inline SEXP readREALSXP(FILE *fin) {
  int length;
  readLength(fin, length);
  SEXP x = PROTECT(Rf_allocVector(REALSXP, length));
  safe_fread((char*)(&REAL(x)[0]), 8, length, fin);
  UNPROTECT(1);
  return x;
}

//read INTEGER object
inline SEXP readINTSXP(FILE *fin) {
  int length;
  readLength(fin, length);
  SEXP x = PROTECT(Rf_allocVector(INTSXP, length));
  safe_fread((char*)(&INTEGER(x)[0]), 4, length, fin);
  UNPROTECT(1);
  return x;
}

//write COMPLEX object
inline SEXP readCPLXSXP(FILE *fin) {
  int length;
  readLength(fin, length);
  SEXP x = PROTECT(Rf_allocVector(CPLXSXP, length));
  safe_fread((char*)(&COMPLEX(x)[0]), 16, length, fin);
  UNPROTECT(1);
  return x;
}

//read RAW object
inline SEXP readRAWSXP(FILE *fin) {
  int length;
  readLength(fin, length);
  SEXP x = PROTECT(Rf_allocVector(RAWSXP, length));
  safe_fread((char*)(&RAW(x)[0]), 1, length, fin);
  UNPROTECT(1);
  return x;
}

//read LOGICAL object
inline SEXP readLGLSXP(FILE *fin) {
  int length;
  readLength(fin, length);
  SEXP x = PROTECT(Rf_allocVector(LGLSXP, length));
  safe_fread((char*)(&LOGICAL(x)[0]), 4, length, fin);
  UNPROTECT(1);
  return x;
}

//read CHAR object
inline SEXP readCHARSXP(FILE *fin) {
  std::vector<BYTE> tempRaw(4);
  std::vector<BYTE> naString(4, 0xff);
  safe_fread((char *) & (tempRaw[0]), 1, 4, fin);
  SEXP _x;
  if (tempRaw == naString) {
    _x = PROTECT(NA_STRING);
  } else {
    std::string x;
    int length;
    fseek(fin, -4, SEEK_CUR);
    safe_fread((char *) & (length), 4, 1, fin);
    x.resize(length);
    safe_fread((char*)(&x[0]), 1, length, fin);
    _x = PROTECT(Rf_mkChar(x.c_str()));
  }
  UNPROTECT(1);
  return _x;
}

//read CHARACTER VECTOR object
inline SEXP readSTRSXP(FILE *fin) {
  int length;
  readLength(fin, length);
  SEXP x = PROTECT(Rf_allocVector(STRSXP, length));
  for (int64_t i = 0; i < length; i++) {
    SEXP temp = PROTECT(readSEXP(fin));
    SET_STRING_ELT(x, i, temp);
    UNPROTECT(1);
  }
  UNPROTECT(1);
  return x;
}

//read a LIST object
inline SEXP readVECSXP(FILE *fin) {
  int length;
  readLength(fin, length);
  SEXP x = PROTECT(Rf_allocVector(VECSXP, length));
  for (int64_t i = 0; i < length; i++) {
    SEXP temp = PROTECT(readSEXP(fin));
    SET_VECTOR_ELT(x, i, temp);
    UNPROTECT(1);
  }
  UNPROTECT(1);
  return x;
}

//read a SYM object
inline SEXP readSYMSXP(FILE *fin) {
  SEXP chars = PROTECT(readSEXP(fin));
  SEXP x = PROTECT(Rf_install(CHAR(chars)));
  UNPROTECT(2);
  return x;
}

//read the head info the object
inline void readHead(FILE *fin, int &type, int &has_attr, int &has_tag, int &has_object, int &level) {
  int head_info;
  safe_fread((char*)(&head_info), 4 , 1, fin);
  type = head_info & 255;
  has_object = (head_info >> 8) & 1;
  has_attr = (head_info >> 9) & 1;
  has_tag =  (head_info >> 10) & 1;
  level = head_info >> 12;
  return;
}

//read object length
inline void readLength(FILE *fin, int &length) {
  safe_fread((char*)(&length), 4, 1, fin);
  return;
}

//read ATTRIBUTE of object
inline void readATTR(SEXP &_x, FILE *fin) {
  SEXP parlist = PROTECT(readSEXP(fin));
  SEXP current_chain = parlist;
  while (true) {
    SEXP chain = PROTECT(readSEXP(fin));
    if (chain != R_NilValue) {
      current_chain = SETCDR(current_chain, chain);
      UNPROTECT_PTR(chain);
    } else {
      UNPROTECT_PTR(chain);
      break;
    }
  }
  SET_ATTRIB(_x, parlist);
  UNPROTECT_PTR(parlist);
  return;
}

//main function of reading a R object
SEXP readSEXP(FILE *fin)
{
  int type, has_attr, has_tag, has_object, level;
  readHead(fin, type, has_attr, has_tag, has_object, level);
  SEXP element = R_NilValue;
  switch (type) {
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
  case LISTSXP  :{
    SEXP tag;
    SEXP el;
    if (has_tag == 1) {
      tag = PROTECT(readSEXP(fin));
    }
    el = PROTECT(readSEXP(fin));
    element = PROTECT(Rf_cons(el, R_NilValue));
    UNPROTECT_PTR(el);
    if (has_tag == 1) {SET_TAG(element, tag); UNPROTECT_PTR(tag);}
  } break;
  }
  if (has_attr == 1) {
    readATTR(element, fin);
  }
  if (has_object == 1) {
    SET_OBJECT(element, 1);
  }
  SETLEVELS(element, level);
  UNPROTECT(1);
  return (element);
}
