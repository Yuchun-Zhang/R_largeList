#include "largeList.h"

inline void getHeadInfo(SEXP _x, int &level, int &object, SEXP &attribute, SEXP &tag){
  attribute = TYPEOF(_x) == CHARSXP ? R_NilValue : ATTRIB(_x);
  level = TYPEOF(_x) == CHARSXP ? LEVELS(_x) & 65502 : LEVELS(_x);
  tag = TYPEOF(_x) == LISTSXP ? TAG(_x) : R_NilValue;
  object = OBJECT(_x);
  return;
}

inline void  writeREALSXP(SEXP _x, FILE *fout){
  writeLength(_x, fout);
  fwrite((char *)(& REAL(_x)[0]),8, Rf_xlength(_x), fout);
  return;
}

inline void  writeNILSXP(FILE *fout){
  BYTE nil[4] = {0xfe,0x00,0x00,0x00};
  fwrite((char *)&(nil[0]), 1, 4, fout);
  return;
}

inline void  writeINTSXP(SEXP _x, FILE *fout){
  writeLength(_x, fout);
  fwrite((char *)(& INTEGER(_x)[0]), 4, Rf_xlength(_x), fout);
  return;
}

inline void  writeCPLXSXP(SEXP _x, FILE *fout){
  writeLength(_x, fout);
  fwrite((char *)(& COMPLEX(_x)[0]), 16, Rf_xlength(_x), fout);
  return;
}

inline void  writeLGLSXP(SEXP _x, FILE *fout){
  writeLength(_x, fout);
  fwrite((char *)(& LOGICAL(_x)[0]), 4, Rf_xlength(_x), fout);
  return;
}

inline void  writeRAWSXP(SEXP _x, FILE *fout){
  writeLength(_x, fout);
  fwrite((char *)(& RAW(_x)[0]), 1, Rf_xlength(_x), fout);
  return;
}
  
inline void  writeSTRSXP(SEXP _x, FILE *fout){
  writeLength(_x, fout);
  for (int64_t i = 0; i < Rf_xlength(_x); i++){
    writeSEXP(STRING_ELT(_x, i), fout);
  }
  return;
}

inline void  writeCHARSXP(SEXP _x, FILE *fout){
  if (_x == NA_STRING ){
    BYTE ffff[4] = {0xff,0xff,0xff,0xff};
    fwrite((char *)&(ffff[0]), 1, 4, fout);
  }else{
    writeLength(_x, fout);
    fwrite(CHAR(_x), 1, Rf_xlength(_x), fout);
  }
  return;
}

inline void  writeVECSXP(SEXP _x, FILE *fout){
  writeLength(_x, fout);
  for (int64_t i = 0; i < Rf_xlength(_x); i++){
    writeSEXP(VECTOR_ELT(_x, i), fout);
  }
  return;
}

inline void  writeATTR(SEXP _x, FILE *fout){
  for(SEXP nxt = _x; nxt!= R_NilValue; nxt = CDR(nxt)) {
    writeSEXP(nxt,fout);
  }
  writeNILSXP(fout);
  return;
}

inline void  writeSYMSXP(SEXP _x, FILE *fout){
  SEXP name = PRINTNAME(_x);
  writeSEXP(name, fout);
  return;
}

inline void  writeHead(SEXP _x, int level, int object, SEXP attribute, SEXP tag, FILE *fout){
  int head = TYPEOF(_x) +
             (object << 8) + 
             ((int)(attribute != R_NilValue) << 9) + 
             ((int)(tag != R_NilValue) << 10) + 
             (level <<12);
  fwrite((char *)(&head), 1, 4, fout);
  return;
}

inline void  writeLength(SEXP _x, FILE *fout){
  int length = LENGTH(_x);
  fwrite((char *)&length,4, 1, fout);
  return;
}

// inline void  writeHeadAndLength(SEXP _x, std::fstream & fout){
//   int level, object;
//   SEXP attribute, tag;
//   getHeadInfo(_x, level, object, attribute, tag);
//   writeHead(_x, level, object, attribute, tag, fout);
//   writeLength(_x, fout);
//   return;
// }

int writeSEXP(SEXP _x, FILE *fout){
  int level, object;
  SEXP attribute, tag;
  getHeadInfo(_x, level, object, attribute, tag);
  if (TYPEOF(_x) != NILSXP){
    writeHead(_x, level, object, attribute, tag, fout);
  }
  switch(TYPEOF(_x)){
  case NILSXP   : { writeNILSXP(fout);  break; }
  case REALSXP  : { writeREALSXP(_x,fout); break; }
  case INTSXP   : { writeINTSXP(_x,fout);  break; }
  case CPLXSXP  : { writeCPLXSXP(_x,fout); break; }
  case STRSXP   : { writeSTRSXP(_x,fout);  break; }
  case CHARSXP  : { writeCHARSXP(_x,fout); break; }
  case LGLSXP   : { writeLGLSXP(_x,fout);  break; }
  case VECSXP   : { writeVECSXP(_x,fout);  break; }
  case RAWSXP   : { writeRAWSXP(_x,fout);  break; }
  case SYMSXP   : { writeSYMSXP(_x,fout);  break; }
  case LISTSXP  : { SEXP el = CAR(_x);
                    SEXP tag = TAG(_x);
                    writeSEXP(tag, fout);
                    writeSEXP(el, fout); 
                    break; }
  default : {throw -1;}
  }
  if (attribute != R_NilValue){
    writeATTR(attribute, fout);
  }
  //Rcout <<"give in finished \n";
  return(true);
}

// RcppExport SEXP unitTest(SEXP x, CharacterVector fileName){
//   std::fstream fout;
//   std::string fname = Rcpp::as<std::string>(fileName);
//   fout.open(fname, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
//   Environment base("package:base");
//   Function serialize = base["serialize"];
//   List emptyList(0);
//   RawVector emptyListrawVec = serialize(emptyList,R_NilValue,wrap(false), wrap(false));
//   fout.write((char *)&(emptyListrawVec[0]),14);
//   writeSEXP(x,fout);
//   fout.close();
//   return(wrap(true));
// }




