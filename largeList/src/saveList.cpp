#include "largeList.h"
using namespace Rcpp;
// [[Rcpp::export]]

RcppExport SEXP saveList(SEXP object, 
                         SEXP file, 
                         SEXP append)
{ 
  //get necessary R functions.
  Environment base("package:base");
  Function serialize = base["serialize"];
  
  //make sure the input object is a list.
  if (TYPEOF(object) != VECSXP){
    throw std::range_error("Object is not a list.");
  }
  
  //declare file io pointers.
  std::string fileName = Rcpp::as<std::string>(file);
  std::fstream fout;
  std::fstream fin;
  
  //if append is false or if the file does not exist, save the file in binary as well
  //as the positions of all elements.
  if (as<bool>(append) == false) {
    List listObject(object);
    int lengthOfList = LENGTH(listObject);
    fout.open(fileName.c_str(), std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
    fout.close();
    fout.open(fileName.c_str(), std::ios_base::binary | std::ios_base::out);
    //use an empty list to generate the binary file head.
    List emptyList(0);
    RawVector emptyListrawVec = serialize(emptyList,R_NilValue,wrap(false), wrap(false));
    fout.write((char *)&(emptyListrawVec[0]),14);
    BYTE seg[4] = {0x13,0x02,0x00,0x00};
    fout.write((char *)&(seg[0]),4);
    fout.write((char *)&(lengthOfList),4);
    
    std::vector<std::pair<std::string, int64_t> > uuidIdx(lengthOfList);
    int64_t lastPosition;
    //Rprintf("Begin to store elements");
    //array headPositions stores the positions of elements.
    for(int i = 0; i < lengthOfList; i++){
      uuidIdx[i].second = fout.tellp();
      writeSEXP(listObject[i],fout);
    }
    lastPosition = fout.tellp();
    
    //get element names
    StringVector namesSXP(lengthOfList);
    getObjectName(listObject, namesSXP);
    
    // writeNameAttrHead(fout);
    // writeNameAttrLength(lengthOfList,fout);
    // writeSTRSXP(namesSXP,fout);
    // writeNameAttrEnd(fout);
    for(int i = 0; i < lengthOfList; i++){
      std::string nameTemp;
      if (namesSXP[i] == NA_STRING){
        nameTemp = "\x00\x00\x00\x00\x00\x00\x00\x00";
      }else{
        nameTemp = Rcpp::as<std::string>(namesSXP[i]);
      }
      uuidIdx[i].first = nameTemp;
      uuidIdx[i].first.resize(NAMELENGTH);
    }
    
    // write first refference table
    for(int i = 0; i < lengthOfList; i++){
      fout.write((char *)&(uuidIdx[i].second),8);
      fout.write(uuidIdx[i].first.c_str(),NAMELENGTH);
    }
    fout.write((char *)&(lastPosition),8);
  
    std::stable_sort(uuidIdx.begin(), uuidIdx.end(), cmp);
    
    for(int i = 0; i < lengthOfList; i++){
      fout.write((char *)&(uuidIdx[i].second),8);
      fout.write(uuidIdx[i].first.c_str(),NAMELENGTH);
    }  
    fout.close();
    return(wrap(true)); 
  }
  else
  {
    if (FILE *ftest = fopen(fileName.c_str(), "r")) {
      fclose(ftest);
    }else{
      stop("File %s does not exist!\n",fileName.c_str());
    }
    
    //if append is TRUE, append the new elememts and their positions.
    List listObject(object);
    int lengthOfList = listObject.size();
    
    //get previous length.
    fin.open(fileName.c_str(), std::ios_base::binary | std::ios_base::in);
    fin.seekg(18, std::ios_base::beg);
    int oldLengthOfList;
    fin.read((char*)(&oldLengthOfList), 4);
    
    fin.seekg(0, std::ios_base::end);
    // int64_t oldFileLength = fin.tellg();
    
    //get previous positions and name.
    std::vector<std::pair<std::string, int64_t> > uuidIdx(lengthOfList+oldLengthOfList);
    fin.seekg(-(8+NAMELENGTH)*2*oldLengthOfList-8, std::ios_base::end);
    
    for(int i = 0; i < oldLengthOfList; i++){
      fin.read((char *)&(uuidIdx[i].second),8);
      uuidIdx[i].first.resize(NAMELENGTH);
      fin.read((char *)&((uuidIdx[i].first)[0]),NAMELENGTH);
    }
    int64_t oldLastPosition;
    fin.read((char *)&(oldLastPosition),8);
    
    //get the second refference table
    std::vector<std::pair<std::string, int64_t> > uuidIdx2Old(oldLengthOfList);
    fin.seekg(-(8+NAMELENGTH)*oldLengthOfList, std::ios_base::end);
    
    for(int i = 0; i < oldLengthOfList; i++){
      fin.read((char *)&(uuidIdx2Old[i].second),8);
      uuidIdx2Old[i].first.resize(NAMELENGTH);
      fin.read((char *)&((uuidIdx2Old[i].first)[0]),NAMELENGTH);
    }
    
    //get previous names.
    // int64_t namesWithoutHeadLength = oldFileLength-(8+NAMELENGTH)*2*oldLengthOfList-8 - oldLastPosition -29;
    // std::vector<BYTE> nameWithoutHeadBytes(namesWithoutHeadLength);
    // fin.seekg(oldLastPosition+29, std::ios_base::beg);
    // fin.read((char*)&(nameWithoutHeadBytes[0]),namesWithoutHeadLength);

    fout.open(fileName.c_str(), std::ios_base::binary | std::ios_base::out | std::ios_base::in);
    //save the new length to file.
    fout.seekp(18, std::ios_base::beg);
    int newLength = oldLengthOfList+lengthOfList;
    fout.write((char *)&(newLength),4);

    //save elements in the new list
    fout.seekp(oldLastPosition, std::ios_base::beg);

    for(int i = 0; i < lengthOfList; i++){
      uuidIdx[i+oldLengthOfList].second = fout.tellp();
      writeSEXP(listObject[i],fout);
    }
    int64_t newLastPosition;
    newLastPosition = fout.tellp();
    

    StringVector namesSXP(lengthOfList);
    getObjectName(listObject, namesSXP);

    // writeNameAttrHead(fout);
    // writeNameAttrLength(newLength,fout);
    // // fout.write((char*)&(nameWithoutHeadBytes[0]),namesWithoutHeadLength-4);
    // writeSTRSXP(namesSXP,fout);
    // writeNameAttrEnd(fout);

    for(int i = 0; i < lengthOfList; i++){
      std::string nameTemp;
      if (namesSXP[i] == NA_STRING){
        nameTemp = "\x00\x00\x00\x00\x00\x00\x00\x00";
      }else{
        nameTemp = Rcpp::as<std::string>(namesSXP[i]);
      }
      uuidIdx[i+oldLengthOfList].first = nameTemp;
      uuidIdx[i+oldLengthOfList].first.resize(NAMELENGTH);
      // Rcout << uuidIdx[i+oldLengthOfList].first <<"\n";
    }

    // for(int i = 0; i <newLength ; i++){
    //   Rcout << uuidIdx[i].second << "\n";
    // }
    
    for(int i = 0; i <newLength ; i++){
      fout.write((char *)&(uuidIdx[i].second),8);
      fout.write(uuidIdx[i].first.c_str(),NAMELENGTH);
    }
    fout.write((char *)&(newLastPosition),8);
    
    std::vector<std::pair<std::string, int64_t> > uuidIdx2New(lengthOfList+oldLengthOfList);
    int currentToInsertIndex = 0;
    int currentOriginIndex = 0;
    int currentNewIndex = 0;
    do {
      if (currentOriginIndex == oldLengthOfList) {
        uuidIdx2New[currentNewIndex] = uuidIdx[oldLengthOfList+currentToInsertIndex];
        currentToInsertIndex ++;
      }else 
      if (currentToInsertIndex == lengthOfList)
      {
        uuidIdx2New[currentNewIndex] = uuidIdx2Old[currentOriginIndex];
        currentOriginIndex ++;
      }else
      if (cmp(uuidIdx2Old[currentOriginIndex], uuidIdx[oldLengthOfList+currentToInsertIndex]) == true)
      {
        uuidIdx2New[currentNewIndex] = uuidIdx2Old[currentOriginIndex];
        currentOriginIndex ++;
      }else
      {
        uuidIdx2New[currentNewIndex] = uuidIdx[oldLengthOfList+currentToInsertIndex];
        currentToInsertIndex ++;
      }
      // Rcout << currentToInsertIndex << " "<<currentOriginIndex<< " "<< currentNewIndex << "\n";
      currentNewIndex ++;
    }while( currentNewIndex < lengthOfList+oldLengthOfList);

    for(int i = 0; i < newLength; i++){
      fout.write((char *)&(uuidIdx2New[i].second),8);
      fout.write(uuidIdx2New[i].first.c_str(),NAMELENGTH);
    }
    
    // std::stable_sort(uuidIdx.begin(), uuidIdx.end(), cmp);
    // for(int i = 0; i < newLength; i++){
    //   fout.write((char *)&(uuidIdx[i].second),8);
    //   fout.write(uuidIdx[i].first.c_str(),NAMELENGTH);
    // }
     fin.close();
    fout.close();
    return(wrap(true)); 
  }
}


