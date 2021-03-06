#include "large_list.h"

extern "C" SEXP modifyInList(SEXP file, SEXP index, SEXP object, SEXP verbose) {

	large_list::ProgressReporter general_reporter;

	//check parameters
	if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("file should be a charater vector of length 1.");
	if (TYPEOF(object) != VECSXP) error("object is not a list.");
	if (Rf_length(object) == 0) error("replacement has length zero.");
	if (TYPEOF(index) != INTSXP &&  TYPEOF(index) != REALSXP && TYPEOF(index) != LGLSXP && TYPEOF(index) != STRSXP)
		error("index should be a NULL, an integer vector, a numeric vector, a logical vector or a character vector.");
	large_list::ConnectionFile connection_file(file);
	large_list::MemorySlot memory_slot;
	try {connection_file.connect(); } catch (std::exception &e) { connection_file.disconnect(); error(e.what());}

	// Rprintf("Begin to deal with index \n");
	// deal with index.
	large_list::ListObject list_object_to_save(object);
	try { list_object_to_save.check(); } catch (std::exception &e) { connection_file.disconnect(); error(e.what());}
	large_list::MetaListObject list_object_origin;
	list_object_origin.readLength(connection_file);
	list_object_origin.readCompressBit(connection_file);
	list_object_to_save.setCompressBit(list_object_origin.getCompressBit());
	large_list::IndexWithValueObject index_object(index, list_object_origin.getLength(), connection_file);
	index_object.setValueLength(list_object_to_save.getLength());
	index_object.setValueIndex();
	index_object.removeInvalid();
	index_object.sort();
	index_object.removeDuplicate();
	// index_object.print();

	if (index_object.getLength() == 0) { return (ScalarLogical(1)); }

	// get original pair.
	large_list::NamePositionTuple pair_origin;
	pair_origin.resize(list_object_origin.getLength());
	pair_origin.read(connection_file);
	pair_origin.readLastPosition(connection_file);

	// get new object length.
	list_object_to_save.calculateSerializedLength(memory_slot);

	// get new pair.
	large_list::NamePositionTuple pair_new(pair_origin);
	int i = 0;
	int j = 0;
	int64_t offset = 0;
	for(j = 0; j < list_object_origin.getLength(); j++) {
		if (offset != 0) {
			// Rprintf("change position %3.0ld to %3.0ld \n", pair_new.getPosition(j), pair_new.getPosition(j) + offset);
			pair_new.setPosition(pair_new.getPosition(j) + offset, j);
		}
		if (j == index_object.getIndex(i)) {
			offset += pair_origin.getPosition(index_object.getIndex(i)) - 
				pair_origin.getPosition(index_object.getIndex(i) + 1) + 
				list_object_to_save.getSerializedLength(index_object.getValueIndex(i));
			if (i < (index_object.getLength() - 1)) i++;
		}
	}
	pair_new.setLastPosition(pair_new.getLastPosition() + offset);    

	// check how many blocks need to be moved 
	int num_of_blocks_to_move = 0;
	for (int i = 0; i < list_object_origin.getLength(); i ++) {
		if (pair_new.getPosition(i) != pair_origin.getPosition(i)) num_of_blocks_to_move++;
	}

	// move the date
	int num_of_blocks_moved = 0;
	large_list::ProgressReporter moving_reporter;
	int64_t first_move_pos = -1;
	for (int i = 0; i < list_object_origin.getLength(); i ++) {
		// Rprintf("index %d first_move_pos %3.0lf \n",i, (double)first_move_pos);
		if (pair_new.getPosition(i) == pair_origin.getPosition(i) && first_move_pos == -1) continue;
		if (pair_new.getPosition(i) < pair_origin.getPosition(i)) {
			connection_file.moveData(pair_origin.getPosition(i), pair_origin.getPosition(i + 1),
									 pair_new.getPosition(i), pair_new.getPosition(i + 1));
			// Rprintf("NORMAL MOVE %3.0ld,%3.0ld,%3.0ld,%3.0ld \n", pair_origin.getPosition(i), pair_origin.getPosition(i + 1),
			// pair_new.getPosition(i), pair_new.getPosition(i + 1));
			// Print progress to console
			num_of_blocks_moved++;
			if (LOGICAL(verbose)[0] == true) moving_reporter.reportProgress(num_of_blocks_moved, num_of_blocks_to_move, "Step1 : Moving Data");
		}
		if (pair_new.getPosition(i) > pair_origin.getPosition(i) && first_move_pos == -1) {
			first_move_pos = i;
		}
		if ((pair_new.getPosition(i + 1) <= pair_origin.getPosition(i + 1) || i == list_object_origin.getLength() - 1 ) && first_move_pos != -1) {
			for (int j = i; j >= first_move_pos; j--) {
				connection_file.moveData(pair_origin.getPosition(j), pair_origin.getPosition(j + 1),
										 pair_new.getPosition(j), pair_new.getPosition(j + 1));
				// Rprintf("RECURSIVE MOVE %3.0ld,%3.0ld,%3.0ld,%3.0ld \n", pair_origin.getPosition(j), pair_origin.getPosition(j + 1),
				// pair_new.getPosition(j), pair_new.getPosition(j + 1));
				// Print progress to console
				num_of_blocks_moved++;
				if (LOGICAL(verbose)[0] == true) moving_reporter.reportProgress(num_of_blocks_moved, num_of_blocks_to_move, "Step1 : Moving Data");
		    }
			first_move_pos = -1;
		}
	}
	// Rprintf("Move Finished \n");


	//write Object
	large_list::ProgressReporter writing_reporter;
	for (int i = 0; i < index_object.getLength(); i ++) {
		connection_file.seekWrite(pair_new.getPosition(index_object.getIndex(i)), SEEK_SET);
		list_object_to_save.write(connection_file, memory_slot, index_object.getValueIndex(i));

		// Print progress to console
		if (LOGICAL(verbose)[0] == true) writing_reporter.reportProgress(i, index_object.getLength(), "Step2 : Writing Data");
	}

	// Rprintf("Write Object Finished \n");

	//write tables
	connection_file.seekWrite(pair_new.getLastPosition(), SEEK_SET);
	pair_new.write(connection_file, true);
	pair_new.sort();
	pair_new.write(connection_file, false);

	// Rprintf("Write Table Finished \n");

	//cut file
	connection_file.cutFile();

	// Print progress to console
	general_reporter.is_long_time_ = moving_reporter.is_long_time_ || writing_reporter.is_long_time_;
	if (LOGICAL(verbose)[0] == true) general_reporter.reportFinish("Modifying Data");

	// Rprintf("Cut file Finished \n");
	return (ScalarLogical(1));
}

extern "C" SEXP modifyNameInList(SEXP file, SEXP index, SEXP names) {
	//check parameters
	if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("file should be a charater vector of length 1.");
	if (TYPEOF(names) != STRSXP && TYPEOF(names) != NILSXP) error("parameter names is neither a character vector nor NULL.");
	if (index != R_NilValue && TYPEOF(index) != INTSXP &&  TYPEOF(index) != REALSXP && TYPEOF(index) != LGLSXP )
		error("index should be a NULL, an integer vector, a numeric vector or a logical vector.");
	large_list::ConnectionFile connection_file(file);
	try {connection_file.connect(); } catch (std::exception &e) { connection_file.disconnect(); error(e.what());}

	// deal with index.
	large_list::MetaListObject list_object_origin;
	list_object_origin.readLength(connection_file);
	list_object_origin.readNameBit(connection_file);
	large_list::IndexWithValueObject index_object(index, list_object_origin.getLength(), connection_file,
												  list_object_origin.getNameBit() ? true : false);
	index_object.setValueLength(Rf_length(names));
	index_object.setValueIndex();
	index_object.sort();
	index_object.removeDuplicate();

	if (index_object.getLength() == 0 && list_object_origin.getNameBit() == true) { return (ScalarLogical(1)); }
	if (index_object.getLength() == 0 && list_object_origin.getNameBit() == false) {
		list_object_origin.setNameBit(true);
		list_object_origin.writeNameBit(connection_file);
		return (ScalarLogical(1));
	}
	large_list::NamePositionTuple pair_origin;
	pair_origin.resize(list_object_origin.getLength());
	pair_origin.read(connection_file);
	pair_origin.readLastPosition(connection_file);

	// if the list has names and parameter names is NULL, remove the name strings and set has_name to 0.
	if (list_object_origin.getNameBit() == true && TYPEOF(names) == NILSXP) {
		for (int i = 0; i < list_object_origin.getLength(); i++) {
			pair_origin.setName(std::string(NAMELENGTH, '\xff'), i);
		}
		list_object_origin.setNameBit(false);
		list_object_origin.writeNameBit(connection_file);
	}
	// if the list has no names and parater names is NULL, do nothing.
	if (list_object_origin.getNameBit() == false && TYPEOF(names) == NILSXP) {}
	// if the list has names and parameter names has some values, modify the names.
	if (list_object_origin.getNameBit() == true && TYPEOF(names) != NILSXP) {
		for (int i = 0; i < index_object.getLength(); i++) {
			pair_origin.setName(large_list::UnitObject::charsxpToString(STRING_ELT(names, index_object.getValueIndex(i))),
								index_object.getIndex(i));
		}
	}
	// if the list has no names and parameter names has some values, modify them and set has_name to 1.
	if (list_object_origin.getNameBit() == false && TYPEOF(names) != NILSXP) {
		for (int i = 0; i < index_object.getLength(); i++) {
			pair_origin.setName(large_list::UnitObject::charsxpToString(STRING_ELT(names, index_object.getValueIndex(i))),
								index_object.getIndex(i));
		}
		list_object_origin.setNameBit(true);
		list_object_origin.writeNameBit(connection_file);
	}

	//write tables
	connection_file.seekWrite(pair_origin.getLastPosition(), SEEK_SET);
	pair_origin.write(connection_file, true);
	pair_origin.sort();
	pair_origin.write(connection_file, false);
	return (ScalarLogical(1));
}

extern "C" SEXP removeFromList(SEXP file, SEXP index, SEXP verbose) {

	//check parameters
	if (TYPEOF(file) != STRSXP || Rf_length(file) > 1) error("file should be a charater vector of length 1.");
	if (TYPEOF(index) != INTSXP &&  TYPEOF(index) != REALSXP && TYPEOF(index) != LGLSXP && TYPEOF(index) != STRSXP)
		error("index should be a NULL, an integer vector, a numeric vector, a logical vector or a character vector.");
	large_list::ConnectionFile connection_file(file);
	try {connection_file.connect(); } catch (std::exception &e) { connection_file.disconnect(); error(e.what());}

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

	//Rprintf("get new pair started!\n");
	// get new pair.
	large_list::NamePositionTuple pair_new(pair_origin);
	int i = 0;
	int j = 0;
	int64_t offset = 0;
	for(j = 0; j < list_object_origin.getLength(); j++) {
		if (offset != 0) {
			pair_new.setPosition(pair_new.getPosition(j) + offset, j);
		}
		if (j == index_object.getIndex(i)) {
			offset += pair_origin.getPosition(index_object.getIndex(i)) - 
					  pair_origin.getPosition(index_object.getIndex(i) + 1);
			if (i < (index_object.getLength() - 1)) i++;
		}
	}
	pair_new.setLastPosition(pair_new.getLastPosition() + offset);
	
	//Rprintf("moving data started!\n");
	//move the data
	large_list::ProgressReporter removing_reporter;
	for (int i = 0; i < list_object_origin.getLength(); i ++) {
		if (pair_new.getPosition(i) < pair_origin.getPosition(i)) {
			connection_file.moveData(pair_origin.getPosition(i), pair_origin.getPosition(i + 1),
									 pair_new.getPosition(i), pair_new.getPosition(i + 1));
			//Rprintf("MOVE %3.0ld,%3.0ld,%3.0ld,%3.0ld \n", pair_origin.getPosition(i), pair_origin.getPosition(i + 1),
			//                 pair_new.getPosition(i), pair_new.getPosition(i + 1));
		}

		// Print progress to console
		if (LOGICAL(verbose)[0] == true) removing_reporter.reportProgress(i, list_object_origin.getLength(), "Removing Data");
	}
	//Rprintf("moving data finished!\n");

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

	// Print progress to console
	if (LOGICAL(verbose)[0] == true) removing_reporter.reportFinish("Removing Data");

	return (ScalarLogical(1));
}

