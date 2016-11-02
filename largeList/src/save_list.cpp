#include "large_list.h"

extern "C" SEXP saveList(SEXP object, SEXP file, SEXP append, SEXP compress) {
  	//check parameters
  	if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("file should be a charater vector of length 1.");
  	if (TYPEOF(object) != VECSXP) error("object is not a list.");
    if (TYPEOF(append) != LGLSXP) error("append should be logical TRUE/FALSE");
    if (TYPEOF(compress) != LGLSXP) error("compress should be logical TRUE/FALSE");

 	large_list::ConnectionFile connection_file(file);
    // Rprintf("Turn object to list_object \n");
  	large_list::ListObject list_object_to_save(object);
  	try { list_object_to_save.check(); } catch (std::exception &e){ connection_file.disconnect(); error(e.what());}

    // append == false
  	if (LOGICAL(append)[0] == false) {
        // Rprintf("Connect to File \n");
  		try {connection_file.create(); } catch (std::exception &e){ connection_file.disconnect(); error(e.what());}
      list_object_to_save.setCompressBit(LOGICAL(compress)[0]);
  		large_list::NamePositionTuple pair(list_object_to_save.getLength());
  		list_object_to_save.writeListHead(connection_file);
  		list_object_to_save.writeLength(connection_file);
  		for (int i = 0; i < list_object_to_save.getLength(); i ++) {
  			pair.setPosition(connection_file, i);
  			list_object_to_save.write(connection_file, i);
  		}
  		pair.setLastPosition(connection_file);
  		pair.setName(list_object_to_save);
  		pair.write(connection_file, true);
  		pair.sort();
  		pair.write(connection_file, false);
  		list_object_to_save.writeNameBit(connection_file);
      list_object_to_save.writeCompressBit(connection_file);
 	} else {
        // Rprintf("append == T \n");
 		try {connection_file.connect(); } catch (std::exception &e){ connection_file.disconnect(); error(e.what());}
 		// get the original pair and length.
 		large_list::MetaListObject list_object_origin;
 		list_object_origin.readLength(connection_file);
 		list_object_origin.readNameBit(connection_file);
    list_object_origin.readCompressBit(connection_file);
 		//Rprintf("Original Length%d \n", list_object_origin.getLength());
  	large_list::NamePositionTuple pair_origin;
  	pair_origin.resize(list_object_origin.getLength());
  	pair_origin.read(connection_file);
  	pair_origin.readLastPosition(connection_file);
  	//Rprintf("Last position%lf \n", (double)pair_origin.getLastPosition());

    // set compress option for list_object_to_save
    list_object_to_save.setCompressBit(list_object_origin.getCompressBit());
  	// save the new objects.
		large_list::NamePositionTuple pair_to_save(list_object_to_save.getLength());
		connection_file.seekWrite(pair_origin.getLastPosition(), SEEK_SET);
		for (int i = 0; i < list_object_to_save.getLength(); i ++) {
          // Rprintf("Save New Object %d \n", i);
			pair_to_save.setPosition(connection_file, i);
			list_object_to_save.write(connection_file, i);
		} 
      pair_to_save.setLastPosition(connection_file);
		pair_to_save.setName(list_object_to_save);
		pair_origin.merge(pair_to_save);

		// save the new pairs.
		pair_origin.write(connection_file, true);
		pair_origin.sort();
		pair_origin.write(connection_file, false);
		list_object_origin.setLength(list_object_origin.getLength() + list_object_to_save.getLength());
    list_object_origin.setNameBit(list_object_origin.getNameBit() + list_object_to_save.getNameBit());
		list_object_origin.writeLength(connection_file);
		list_object_origin.writeNameBit(connection_file);
 	}
  	return (ScalarLogical(1));
}

extern "C" SEXP checkList(SEXP object) {
  if (TYPEOF(object) != VECSXP) error("object is not a list.");
  large_list::ListObject list_object_to_save(object);
  try { list_object_to_save.check(); } catch (std::exception &e){error(e.what());}
  return (ScalarLogical(1));
}