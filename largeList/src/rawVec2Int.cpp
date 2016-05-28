#include "largeList.h"
using namespace Rcpp;
long long int rawVec2Int(SEXP _rawVec, int bitLength){
  Rcpp::RawVector rawVec(_rawVec);
  int _bit[bitLength];
  for (int i = 0;i < bitLength; i++){
    _bit[i] = rawVec[i];
  }
  long long int length = 0;
  for(int i = 0; i < bitLength; i ++){
    length = length + pow(256,bitLength-i-1)*_bit[i];
  }
  return(length);
}