#include "large_list.h"

extern "C" SEXP readList(SEXP file, SEXP index) {
    //check parameters
    if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("File should be a charater vector of length 1.");
    if (index != R_NilValue && TYPEOF(index) != INTSXP &&  TYPEOF(index) != REALSXP && TYPEOF(index) != LGLSXP && TYPEOF(index) != STRSXP)
        error("Index should be a NULL, an integer vector, a numeric vector, a logical vector or a character vector.");
    large_list::ConnectionFile connection_file(file);
    try {connection_file.connect(); } catch (std::exception &e) { connection_file.~ConnectionFile(); error(e.what());}

    large_list::MetaListObject list_object_in_file;
    large_list::ListObject list_object_to_output;
    list_object_in_file.readLength(connection_file);
    // Rprintf("Getting Index \n");
    large_list::IndexObject index_object(index, list_object_in_file.getLength(), connection_file);
    // Rprintf("Getting Pair \n");
    index_object.readPair(connection_file);
    //index_object.print(2);
    list_object_to_output.resize(index_object.getLength());
    for (int i = 0; i < index_object.getLength(); i ++) {
        connection_file.seekRead(index_object.getPosition(i), SEEK_SET);
        if (index_object.getIndex(i) != R_NaInt) {
            // Rprintf("Reading Object %d \n", i);
            list_object_to_output.read(connection_file, i);
        } else {
            list_object_to_output.set(R_NilValue, i);
        }
        list_object_to_output.setName(index_object.getName(i), i);
    }
    // Rprintf("Assembling \n");
    SEXP output_list = PROTECT(list_object_to_output.assembleRList(connection_file));
    UNPROTECT_PTR(output_list);
    return (output_list);
    // return(ScalarInteger(1));
}

extern "C" SEXP getListLength(SEXP file) {
    //check parameters
    if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("File should be a charater vector of length 1.");
    large_list::ConnectionFile connection_file(file);
    try {connection_file.connect(); } catch (std::exception &e) { connection_file.~ConnectionFile(); error(e.what());}
    large_list::MetaListObject list_object_in_file;
    list_object_in_file.readLength(connection_file);
    return(ScalarInteger(list_object_in_file.getLength()));
}

extern "C" SEXP getListName(SEXP file) {
    //check parameters
    if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("File should be a charater vector of length 1.");
    large_list::ConnectionFile connection_file(file);
    try {connection_file.connect(); } catch (std::exception &e) { connection_file.~ConnectionFile(); error(e.what());}
    large_list::MetaListObject list_object_in_file;
    list_object_in_file.readLength(connection_file);
    list_object_in_file.readNameBit(connection_file);
    if (list_object_in_file.getNameBit() == false) {
        return (R_NilValue);
    }
    large_list::NamePositionPair pair_origin;
    pair_origin.resize(list_object_in_file.getLength());
    pair_origin.read(connection_file);

    SEXP names_sxp = PROTECT(Rf_allocVector(STRSXP, list_object_in_file.getLength()));
    std::string na_string(NAMELENGTH, '\xff');
    for (int i = 0 ; i < list_object_in_file.getLength(); i++) {
      pair_origin.getName(i) == na_string ?
      SET_STRING_ELT(names_sxp, i, NA_STRING) :
      SET_STRING_ELT(names_sxp, i, Rf_mkChar(pair_origin.getName(i).c_str()));
    }
    UNPROTECT(1);
    return(names_sxp);
}

extern "C" SEXP checkFileAndVersionExternal(SEXP file) {
    if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("File should be a charater vector of length 1.");
    large_list::ConnectionFile connection_file(file);
    try {connection_file.connect(); } catch (std::exception &e) { connection_file.~ConnectionFile(); error(e.what());}
    return (ScalarLogical(1));
}

