#include "largeList.h"
using namespace Rcpp;
// [[Rcpp::export]]

RcppExport SEXP saveLargeList(SEXP object, 
                              SEXP fileName, 
                              SEXP append)
{ 
  Rprintf("Function begins");
  //get necessary R functions.
  Environment base("package:base");
  Function serialize = base["serialize"];
  Function writeBin = base["writeBin"];
  Function readBin = base["readBin"];
  Function seek = base["seek"];
  Function file = base["file"];
  Function close = base["close"];
  Function fileExists = base["file.exists"];
  
  //make sure the input object is a list.
  if (TYPEOF(object) != VECSXP){
    throw std::range_error("Object is not a list.");
  }
  
  //if append is false or if the file does not exist, save the file in binary as well
  //as the positions of all elements.
  if (as<bool>(append) == false || as<bool>(fileExists(fileName)) == false) {
    Rcpp::List listObject(object);
    
    //use an empty list to generate the binary file head.
    Rcpp::List emptyList(0);
    long long int lengthOfList = (long long int)listObject.size();
    long long int headPositions[lengthOfList+1];
    Rcpp::RawVector emptyListrawVec = serialize(emptyList,R_NilValue);
    
    //transform the lengthOfList into binary and save after file head.
    Rcpp::RawVector lengthRawVec= int2RawVec(lengthOfList);
    for (long long int i=0; i<4; i++){
      emptyListrawVec[i+18] = lengthRawVec[i];
    }
    SEXP icon = file(fileName,"wb");
    writeBin(emptyListrawVec, icon); 
    close(icon);
    
    Rprintf("Begin to store elements");
    //array headPositions stores the positions of elements.
    headPositions[0] = (long long int)emptyListrawVec.length();
    for (long long int i = 0; i < lengthOfList; i++){
      //transform element i into binary.
      Rcpp::RawVector rawVec = serialize(listObject[i],R_NilValue);
      //exclude the header.
      Rcpp::IntegerVector index(rawVec.size()-14);
      for(long long int j = 14; j < rawVec.size(); j++){
        index[j-14]=j;
      }
      icon = file(fileName,"ab");
      writeBin(rawVec[index], icon);
      close(icon);
      //generate next position.
      headPositions[i+1] = headPositions[i]+rawVec.length()-14;
      Rprintf("headposition %lf \n",(double)headPositions[i]);
    }
    
    //save positions to file. 
    icon = file(fileName,"ab");
    for (long long int i = 0; i < lengthOfList+1; i++){
      writeBin(int2RawVec(headPositions[i], 8), icon);
    }
    close(icon);
    return(Rcpp::wrap(true)); 
  }
  else
  {
    //if append is TRUE, append the new elememts and their positions.
    SEXP iconRead = file(fileName,"rb");
    Rcpp::List listObject(object);
    long long int lengthOfList = (long long int)listObject.size();
    
    //get previous length.
    seek(iconRead,18,"start","read");
    Rcpp::RawVector oldlengthRawVec = readBin(iconRead,"raw",4);
    close(iconRead);
    
    //get previous positions in binary as oldHeadPositionsRawVec.
    long long int oldLengthOfList = rawVec2Int(oldlengthRawVec);
    iconRead = file(fileName,"rb");
    seek(iconRead,-8*(oldLengthOfList+1),"end","read");
    Rcpp::RawVector oldHeadPositionsRawVec = readBin(iconRead,"raw",8*(oldLengthOfList+1));
    close(iconRead);
    
    
    SEXP iconWrite = file(fileName,"r+b");
    //save the new length to file. 
    seek(iconWrite,18,"start","write");
    Rcpp::RawVector newLengthRawVec= int2RawVec(oldLengthOfList+lengthOfList);
    writeBin(newLengthRawVec, iconWrite); 
    close(iconWrite);
    
    //save elements in the new list 
    long long int headPositions[lengthOfList+1];
    
    //get the last position
    Rcpp::RawVector lastHeadPositionRawVec(8);
    for (long long int i = 0; i < 8; i++){
      lastHeadPositionRawVec[i]=oldHeadPositionsRawVec[8*oldLengthOfList+i];
    }
    long long int lastHeadPosition = rawVec2Int(lastHeadPositionRawVec,8);
    
    //the first headPosition is the last position of the previous list.
    headPositions[0] = lastHeadPosition;
    Rcpp::NumericVector positionSXP;
    for (long long int i = 0; i < lengthOfList; i++){
      Rcpp::RawVector rawVec = serialize(listObject[i],R_NilValue);
      Rcpp::IntegerVector index(rawVec.size()-14);
      for(long long int j = 14; j < rawVec.size(); j++){
        index[j-14]=j;
      }
      iconWrite = file(fileName,"r+b");
      positionSXP = Rcpp::wrap((double)headPositions[i]);
      seek(iconWrite, positionSXP, "start","write");
      writeBin(rawVec[index], iconWrite);
      close(iconWrite);
      headPositions[i+1] = headPositions[i]+rawVec.length()-14;
      //Rprintf("headposition %lf \n",(double)headPositions[i]);
    }
    
    //save the previous positions.
    iconWrite = file(fileName,"r+b");
    positionSXP = Rcpp::wrap((double)headPositions[lengthOfList]);
    seek(iconWrite, positionSXP, "start","write");
    writeBin(oldHeadPositionsRawVec,iconWrite);
    
    //save the new positions.
    for (long long int i = 1; i < lengthOfList+1; i++){
      writeBin(int2RawVec(headPositions[i], 8), iconWrite);
    }
    close(iconWrite);
    return(Rcpp::wrap(true)); 
  }
}


