#include "largeList.h"

//take length of a CHARACTER VECTOR object
inline void  lengthOfSTRSXP(SEXP _x, int64_t &length) {
  length += 4;
  for (int64_t i = 0; i < Rf_xlength(_x); i++) {
    lengthOfSEXP(STRING_ELT(_x, i), length);
  }
  return;
}

//take length of a LIST object
inline void  lengthOfVECSXP(SEXP _x, int64_t &length) {
  length += 4;
  for (int64_t i = 0; i < Rf_xlength(_x); i++) {
    lengthOfSEXP(VECTOR_ELT(_x, i), length);
  }
  return;
}

//take length of ATTRIBUTE of an object
inline void  lengthOfATTR(SEXP _x, int64_t &length) {
  for (SEXP nxt = _x; nxt != R_NilValue; nxt = CDR(nxt)) {
    lengthOfSEXP(nxt, length);
  }
  length += 4;
  return;
}

//take length of a SYM object
inline void  lengthOfSYMSXP(SEXP _x, int64_t &length) {
  SEXP name = PRINTNAME(_x);
  lengthOfSEXP(name, length);
  return;
}


//main function of taking length of a R object to file
int lengthOfSEXP(SEXP _x, int64_t &length) {
  int level, object;
  SEXP attribute, tag;
  getHeadInfo(_x, level, object, attribute, tag);
  if (TYPEOF(_x) != NILSXP) {
    length += 4;
  }
  switch (TYPEOF(_x)) {
  case NILSXP   : { length += 4;  break; }
  case REALSXP  : { length += 8 * Rf_xlength(_x) + 4; break; }
  case INTSXP   : { length += 4 * Rf_xlength(_x) + 4;  break; }
  case CPLXSXP  : { length += 16 * Rf_xlength(_x) + 4; break; }
  case STRSXP   : { lengthOfSTRSXP(_x, length);  break; }
  case CHARSXP  : { _x == NA_STRING ?  length += 4 : length += 1 * Rf_xlength(_x) + 4; break; }
  case LGLSXP   : { length += 4 * Rf_xlength(_x) + 4;  break; }
  case VECSXP   : { lengthOfVECSXP(_x, length);  break; }
  case RAWSXP   : { length += 1 * Rf_xlength(_x) + 4;  break; }
  case SYMSXP   : { lengthOfSYMSXP(_x, length);  break; }
  case LISTSXP  : {
    SEXP el = CAR(_x);
    SEXP tag = TAG(_x);
    lengthOfSEXP(tag, length);
    lengthOfSEXP(el, length);
    break;
  }
  default : {throw - 1;}
  }
  if (attribute != R_NilValue) {
    lengthOfATTR(attribute, length);
  }
  return (1);
}