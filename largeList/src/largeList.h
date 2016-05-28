#include <Rcpp.h> 
RcppExport SEXP int2RawVec(long long int, int  bitLength = 4);
long long int rawVec2Int(SEXP, int  bitLength = 4);