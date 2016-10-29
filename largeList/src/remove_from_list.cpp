#include "large_list.h"

extern "C" SEXP removeFromList(SEXP file, SEXP index) {
	//check parameters
  	if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("file should be a charater vector of length 1.");
  	if (TYPEOF(index) != INTSXP &&  TYPEOF(index) != REALSXP && TYPEOF(index) != LGLSXP && TYPEOF(index) != STRSXP)
    	error("index should be a NULL, an integer vector, a numeric vector, a logical vector or a character vector.");
  	large_list::ConnectionFile connection_file(file);
  	try {connection_file.connect(); } catch (std::exception &e){ connection_file.~ConnectionFile(); error(e.what());}

    // deal with index.
  	large_list::MetaListObject list_object_origin;
  	list_object_origin.readLength(connection_file);
  	large_list::IndexObject index_object(index, list_object_origin.getLength(), connection_file);
    index_object.removeInvalid();
    index_object.sort();
    index_object.removeDuplicate();

    if (index_object.getLength() == 0) { return (ScalarLogical(1)); }

    // get original pair.
    large_list::NamePositionTuple pair_origin;
    pair_origin.resize(list_object_origin.getLength());
    pair_origin.read(connection_file);
    pair_origin.readLastPosition(connection_file);

    // get new pair.
    large_list::NamePositionTuple pair_new(pair_origin);
    for (int i = 0; i < index_object.getLength(); i ++) {
      for (int j = index_object.getIndex(i) + 1; j < list_object_origin.getLength(); j++) {
        // Rprintf("i %d, j %d, \n", i, j);
        // pair_new.print(j);
        // pair_origin.print(index_object.getIndex(i));
        // pair_origin.print(index_object.getIndex(i) +1);
        pair_new.setPosition(pair_new.getPosition(j) + pair_origin.getPosition(index_object.getIndex(i)) - 
                             pair_origin.getPosition(index_object.getIndex(i) + 1), j);
        //new_positions[j] += pair[index_num[i]].second - pair[index_num[i] + 1].second;
      }
      pair_new.setLastPosition(pair_new.getLastPosition() + pair_origin.getPosition(index_object.getIndex(i)) - 
                             pair_origin.getPosition(index_object.getIndex(i) + 1));
    }

    //move the data
    for (int i = 0; i < list_object_origin.getLength(); i ++) {
      if (pair_new.getPosition(i) < pair_origin.getPosition(i)) {
        connection_file.moveData(pair_origin.getPosition(i), pair_origin.getPosition(i + 1), 
                         pair_new.getPosition(i), pair_new.getPosition(i + 1));
        //Rprintf("MOVE %3.0ld,%3.0ld,%3.0ld,%3.0ld \n", pair_origin.getPosition(i), pair_origin.getPosition(i + 1), 
        //                 pair_new.getPosition(i), pair_new.getPosition(i + 1));
      }
    }

    //write length and name bit
    list_object_origin.setLength(list_object_origin.getLength() - index_object.getLength());
    list_object_origin.writeLength(connection_file);

    //write two tables
    pair_new.remove(index_object);
    connection_file.seekWrite(pair_new.getLastPosition(), SEEK_SET);
    pair_new.write(connection_file, true);
    pair_new.sort();
    pair_new.write(connection_file, false);

    //cut file
    connection_file.cutFile();

  	return (ScalarLogical(1));
}