#include "largeList.h"

int getHeadInfo(SEXP _x, int &level, int &object, SEXP &attribute, SEXP &tag){
  level = LEVELS(_x);
  if (TYPEOF(_x) == CHARSXP) {
    attribute = R_NilValue;
    level = level & 65502;
  }else{
    attribute = ATTRIB(_x);
  }
  if (TYPEOF(_x) == LISTSXP) {
    tag = TAG(_x);
  }else{
    tag = R_NilValue;
  }
  object = OBJECT(_x);
  return(true);
}

int writeREALSXP(SEXP _x, std::fstream &fout){
  fout.write((char *)(& REAL(_x)[0]),8*Rf_xlength(_x));
  return(true);
}

int writeNILSXP(std::fstream &fout){
  BYTE nil[4] = {0xfe,0x00,0x00,0x00};
  fout.write((char *)&(nil[0]),4);
  return(true);
}

int writeINTSXP(SEXP _x, std::fstream &fout){
  fout.write((char *)(& INTEGER(_x)[0]),4*Rf_xlength(_x));
  return(true);
}

int writeLGLSXP(SEXP _x, std::fstream &fout){
  fout.write((char *)(& LOGICAL(_x)[0]),4*Rf_xlength(_x));
  return(true);
}

int writeRAWSXP(SEXP _x, std::fstream &fout){
  fout.write((char *)(& RAW(_x)[0]),Rf_xlength(_x));
  return(true);
}
  
int writeSTRSXP(SEXP _x, std::fstream &fout){
  for (int64_t i = 0; i < Rf_xlength(_x); i++){
    writeSEXP(STRING_ELT(_x, i), fout);
  }
  return(true);
}

int writeCHARSXP(SEXP _x, std::fstream &fout){
  if (_x == NA_STRING ){
    BYTE ffff[4] = {0xff,0xff,0xff,0xff};
    fout.write((char *)&(ffff[0]),4);
  }else{
    fout.write(CHAR(_x),Rf_xlength(_x));
  }
  return(true);
}

int writeVECSXP(SEXP _x, std::fstream &fout){
  for (int64_t i = 0; i < Rf_xlength(_x); i++){
    writeSEXP(VECTOR_ELT(_x, i), fout);
  }
  return(true);
}

int writeATTR(SEXP _x, std::fstream &fout){
  for(SEXP nxt = _x; nxt!= R_NilValue; nxt = CDR(nxt)) {
    writeSEXP(nxt,fout);
  }
  writeNILSXP(fout);
  return(true);
}

int writeSYMSXP(SEXP _x, std::fstream &fout){
  SEXP name = PRINTNAME(_x);
  writeSEXP(name, fout);
  return(true);
}

int writeHead(SEXP _x, int level, int object, SEXP attribute, SEXP tag, std::fstream & fout){
  int head = TYPEOF(_x) +
             (object << 8) + 
             ((int)(attribute != R_NilValue) << 9) + 
             ((int)(tag != R_NilValue) << 10) + 
             (level <<12);
  fout.write((char *)(&head),4);
  return(true);
}

int writeLength(SEXP _x, std::fstream & fout){
  int length = LENGTH(_x);
  fout.write((char *)&length,4);
  return(true);
}

int writeHeadAndLength(SEXP _x, std::fstream & fout){
  int level, object;
  SEXP attribute, tag;
  getHeadInfo(_x, level, object, attribute, tag);
  writeHead(_x, level, object, attribute, tag, fout);
  writeLength(_x, fout);
  return(true);
}

int writeSEXP(SEXP _x, std::fstream &fout){
  int level, object;
  SEXP attribute, tag;
  getHeadInfo(_x, level, object, attribute, tag);
  if (TYPEOF(_x) != NILSXP){
    writeHead(_x, level, object, attribute, tag, fout);
  }
  if (TYPEOF(_x) != NILSXP && TYPEOF(_x) != LISTSXP && TYPEOF(_x) != SYMSXP && _x != NA_STRING){
    writeLength(_x, fout);
  }
  switch(TYPEOF(_x)){
  case NILSXP   : writeNILSXP(fout);  break;
  case REALSXP  : writeREALSXP(_x,fout); break;
  case INTSXP   : writeINTSXP(_x,fout);  break;
  case STRSXP   : writeSTRSXP(_x,fout);  break;
  case CHARSXP  : writeCHARSXP(_x,fout); break;
  case LGLSXP   : writeLGLSXP(_x,fout);  break;
  case VECSXP   : writeVECSXP(_x,fout);  break;
  case RAWSXP   : writeRAWSXP(_x,fout);  break;
  case SYMSXP   : writeSYMSXP(_x,fout);  break;
  case LISTSXP  :     
    SEXP el = CAR(_x);
    SEXP tag = TAG(_x);
    writeSEXP(tag, fout);
    writeSEXP(el, fout); break;
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




