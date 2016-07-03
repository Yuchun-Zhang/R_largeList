#include "largeList.h"
using namespace Rcpp;

// [[Rcpp::export]]
RcppExport SEXP getListName(SEXP file)
{
  std::string fileName = Rcpp::as<std::string>(file);
  std::fstream fin;
  fin.open(fileName.c_str(), std::ios_base::binary | std::ios_base::in);
  
  //get length of List
  fin.seekg(18, std::ios_base::beg);
  int lengthOfList;
  fin.read((char*)&(lengthOfList), 4);

  //get names
  SEXP namesSXP = PROTECT(Rf_allocVector(STRSXP, lengthOfList));
  std::string name;
  for(int i = 0 ; i < lengthOfList; i++){
    fin.seekg(-(8+NAMELENGTH)*lengthOfList -(8+NAMELENGTH)*(lengthOfList-i), std::ios_base::end);
    fin.read(&(name[0]),NAMELENGTH);
    // Rcout << "name " << name.c_str() <<"\n";
    SET_STRING_ELT(namesSXP, i, Rf_mkChar(name.c_str()));
  }
  UNPROTECT(1);
  return(namesSXP);
}
