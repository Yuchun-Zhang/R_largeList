#include "largeList.h"

//check a LIST object
inline void  checkVECSXP(SEXP _x) {
  for (int64_t i = 0; i < Rf_xlength(_x); i++) {
    checkSEXP(VECTOR_ELT(_x, i));
  }
  return;
}

//check ATTRIBUTE of an object
inline void  checkATTR(SEXP _x) {
  for (SEXP nxt = _x; nxt != R_NilValue; nxt = CDR(nxt)) {
    checkSEXP(nxt);
  }
  return;
}

//check a SYM object
inline void  checkSYMSXP(SEXP _x) {
  SEXP name = PRINTNAME(_x);
  checkSEXP(name);
  return;
}

//main function of checking a R object to file
int checkSEXP(SEXP _x) {
  int level, object;
  SEXP attribute, tag;
  getHeadInfo(_x, level, object, attribute, tag);
  switch (TYPEOF(_x)) {
  case NILSXP   : { break; }
  case REALSXP  : { break; }
  case INTSXP   : { break; }
  case CPLXSXP  : { break; }
  case STRSXP   : { break; }
  case CHARSXP  : { break; }
  case LGLSXP   : { break; }
  case VECSXP   : { checkVECSXP(_x);  break; }
  case RAWSXP   : { break; }
  case SYMSXP   : { checkSYMSXP(_x);  break; }
  case LISTSXP  : {
    SEXP el = CAR(_x);
    SEXP tag = TAG(_x);
    checkSEXP(tag);
    checkSEXP(el);
    break;
  }
  default : {error("Data type not supported. Please check ?largeList");}
  }
  if (attribute != R_NilValue) {
    checkATTR(attribute);
  }
  return (1);
}