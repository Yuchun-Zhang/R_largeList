#include "largeList.h"

//get the info of the object, e.g. level, is a object?, attribute, tag. 
inline void getHeadInfo(SEXP _x, int &level, int &object, SEXP &attribute, SEXP &tag){
  attribute = TYPEOF(_x) == CHARSXP ? R_NilValue : ATTRIB(_x);
  level = TYPEOF(_x) == CHARSXP ? LEVELS(_x) & 65502 : LEVELS(_x);
  tag = TYPEOF(_x) == LISTSXP ? TAG(_x) : R_NilValue;
  object = OBJECT(_x);
  return;
}

//write a REAL object.
inline void  writeREALSXP(SEXP _x, FILE *fout){
  writeLength(_x, fout);
  fwrite((char *)(& REAL(_x)[0]),8, Rf_xlength(_x), fout);
  return;
}

//write a  NULL object.
inline void  writeNILSXP(FILE *fout){
  BYTE nil[4] = {0xfe,0x00,0x00,0x00};
  fwrite((char *)&(nil[0]), 1, 4, fout);
  return;
}

//write a INTEGER object
inline void  writeINTSXP(SEXP _x, FILE *fout){
  writeLength(_x, fout);
  fwrite((char *)(& INTEGER(_x)[0]), 4, Rf_xlength(_x), fout);
  return;
}

//write a COMPLEX object
inline void  writeCPLXSXP(SEXP _x, FILE *fout){
  writeLength(_x, fout);
  fwrite((char *)(& COMPLEX(_x)[0]), 16, Rf_xlength(_x), fout);
  return;
}

//write a LOGICAL object
inline void  writeLGLSXP(SEXP _x, FILE *fout){
  writeLength(_x, fout);
  fwrite((char *)(& LOGICAL(_x)[0]), 4, Rf_xlength(_x), fout);
  return;
}

//write a RAW object
inline void  writeRAWSXP(SEXP _x, FILE *fout){
  writeLength(_x, fout);
  fwrite((char *)(& RAW(_x)[0]), 1, Rf_xlength(_x), fout);
  return;
}
  
//write a CHARACTER VECTOR object
inline void  writeSTRSXP(SEXP _x, FILE *fout){
  writeLength(_x, fout);
  for (int64_t i = 0; i < Rf_xlength(_x); i++){
    writeSEXP(STRING_ELT(_x, i), fout);
  }
  return;
}

//write a CHAR object
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

//write a LIST object
inline void  writeVECSXP(SEXP _x, FILE *fout){
  writeLength(_x, fout);
  for (int64_t i = 0; i < Rf_xlength(_x); i++){
    writeSEXP(VECTOR_ELT(_x, i), fout);
  }
  return;
}

//write ATTRIBUTE of an object
inline void  writeATTR(SEXP _x, FILE *fout){
  for(SEXP nxt = _x; nxt!= R_NilValue; nxt = CDR(nxt)) {
    writeSEXP(nxt,fout);
  }
  writeNILSXP(fout);
  return;
}

//write a SYM object
inline void  writeSYMSXP(SEXP _x, FILE *fout){
  SEXP name = PRINTNAME(_x);
  writeSEXP(name, fout);
  return;
}

//write the object Info. 
inline void  writeHead(SEXP _x, int level, int object, SEXP attribute, SEXP tag, FILE *fout){
  int head = TYPEOF(_x) +
             (object << 8) + 
             ((int)(attribute != R_NilValue) << 9) + 
             ((int)(tag != R_NilValue) << 10) + 
             (level <<12);
  fwrite((char *)(&head), 1, 4, fout);
  return;
}

//write the length of object
inline void  writeLength(SEXP _x, FILE *fout){
  int length = LENGTH(_x);
  fwrite((char *)&length,4, 1, fout);
  return;
}

//main function of writing a R object to file
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
