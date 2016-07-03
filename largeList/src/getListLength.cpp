#include "largeList.h"
using namespace Rcpp;

// [[Rcpp::export]]
RcppExport SEXP getListLength(SEXP file){
  std::string fileName = Rcpp::as<std::string>(file);
  std::fstream fin;
  fin.open(fileName.c_str(), std::ios_base::binary | std::ios_base::in);
  
  //get length of List
  fin.seekg(18, std::ios_base::beg);
  int lengthOfList;
  fin.read((char*)&(lengthOfList), 4);
  return(wrap(lengthOfList));
}