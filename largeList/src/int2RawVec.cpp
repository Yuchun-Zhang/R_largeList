#include "largeList.h"
#include <math.h>
using namespace Rcpp;

RcppExport SEXP int2RawVec(long long int length, int bitLength){
  //Environment base("package:base");
  //Function asRaw = base["as.raw"];
  int _bit[bitLength];
  long long int remainLength = length;
  for (int i = 0; i < bitLength; i++){
    _bit[i] = remainLength / pow(256,bitLength -i -1);
    remainLength = remainLength-_bit[i]* pow(256,bitLength -i -1);
  }
  Rcpp::RawVector lengthRawVec(bitLength);
  for (int i = 0;i < bitLength;i++){
    lengthRawVec[i] = _bit[i];
  }
  return(lengthRawVec);
}