#include "largeList.h"

extern "C" SEXP saveList(SEXP object, SEXP file, SEXP append)
{ 
  //check parameters
  if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("File should be a charater vector of length 1.\n");
  if (TYPEOF(object) != VECSXP) error("Object is not a list.\n");
  
  //declare file io pointers.
  const char *fileName = getFullPath(file);
  FILE *fout;
  FILE *fin;
  
  //if append is false or if the file does not exist, save the file in binary as well
  //as the positions of all elements.
  if (LOGICAL(append)[0] == false) {
    int lengthOfList =Rf_xlength(object);
    fout = fopen(fileName, "wb");
    std::vector<std::pair<std::string, int64_t> > itemIdx(lengthOfList);
    int64_t lastPosition;
    
    writeVersion(fout);
    const BYTE listHead[5] = {0x13,0x00,0x00,0x00};
    fwrite((char *)&listHead[0], 1, 4, fout);
    fwrite((char *)&(lengthOfList), 4, 1, fout);

    //array itemIdx stores the positions of elements.
    for(int i = 0; i < lengthOfList; i++){
      itemIdx[i].second = ftell(fout);
      writeSEXP(VECTOR_ELT(object, i),fout);
    }
    lastPosition = ftell(fout);
    
    //get element names
    SEXP namesSXP = getObjectName(object);

    for(int i = 0; i < lengthOfList; i++){
      std::string nameTemp = STRING_ELT(namesSXP,i) == NA_STRING ? 
                            "\x00\x00\x00\x00\x00\x00\x00\x00" :  
                            CHAR(STRING_ELT(namesSXP, i));
      itemIdx[i].first = nameTemp;
      itemIdx[i].first.resize(NAMELENGTH);
    }
    
    // write first refference table
    writeItemIdx(itemIdx, fout,lengthOfList);
    fwrite((char *)&(lastPosition), 8, 1, fout);
    
    //sort and write second table
    std::stable_sort(itemIdx.begin(), itemIdx.end(), cmp);
    writeItemIdx(itemIdx, fout,lengthOfList);
    
    fclose(fout);
    return(ScalarLogical(1)); 
  }
  else
  {
    //if append is TRUE, append the new elememts and their positions.
    if(checkFile(fileName) == false) error("File does not exist.\n");
    int lengthOfListAppend = Rf_length(object);

    //get previous length.
    fin = fopen(fileName, "rb");
    fseek(fin, 18, SEEK_SET);
    int lengthOfListOld;
    fread((char*)(&lengthOfListOld), 4, 1, fin);
    int64_t lastPositionOld;
    int64_t lastPositionNew;
    int lengthOfListNew = lengthOfListOld+lengthOfListAppend;

    std::vector<std::pair<std::string, int64_t> > itemIdx(lengthOfListOld);
    
    //get previous first table
    fseek(fin, -(8+NAMELENGTH)*2*lengthOfListOld-8, SEEK_END);
    readItemIdx(itemIdx, fin, lengthOfListOld);
    fread((char *)&(lastPositionOld), 8, 1, fin);
    
    //save the new length to file.
    fout = fopen(fileName, "r+b");
    fseek(fout, 18, SEEK_SET);
    fwrite((char *)&(lengthOfListNew), 4, 1, fout);

    //save elements in the new list
    fseek(fout, lastPositionOld, SEEK_SET);

    for(int i = 0; i < lengthOfListAppend; i++){
      itemIdx[i+lengthOfListOld].second = ftell(fout);
      writeSEXP(VECTOR_ELT(object,i),fout);
    }
    lastPositionNew = ftell(fout);

    // get the new names 
    SEXP namesSXP = getObjectName(object);
    for(int i = 0; i < lengthOfListAppend; i++){
      std::string nameTemp = STRING_ELT(namesSXP,i) == NA_STRING ?
                             "\x00\x00\x00\x00\x00\x00\x00\x00" :
                             CHAR(STRING_ELT(namesSXP, i));
      itemIdx[i+lengthOfListOld].first = nameTemp;
      itemIdx[i+lengthOfListOld].first.resize(NAMELENGTH);
    }

    //write the first table 
    writeItemIdx(itemIdx, fout, lengthOfListNew);
    fwrite((char *)&(lastPositionNew), 8, 1, fout);
    
    //merge to generate the second table and write 
    std::stable_sort(itemIdx.begin(), itemIdx.end(), cmp);
    writeItemIdx(itemIdx, fout, lengthOfListNew);

    fclose(fin);
    fclose(fout);
    return(ScalarLogical(1)); 
   }
}


