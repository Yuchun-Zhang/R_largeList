#include "large_list.h"
namespace large_list {
	//-------------------------- MetaListObject ------------------------
	MetaListObject::MetaListObject () {
		length_ = 0;
		has_name_ = false;
	}

	MetaListObject::MetaListObject (int length) {
		length_ = length;
		has_name_ = false;
	}

	MetaListObject::~MetaListObject () {}

	void MetaListObject::writeLength (ConnectionFile & connection_file) {
		connection_file.seekWrite(LENGTH_POSITION, SEEK_SET);
		connection_file.write(&length_, 4, 1);
		return;
	}

	void MetaListObject::readLength (ConnectionFile & connection_file) {
		connection_file.seekRead(LENGTH_POSITION, SEEK_SET);
		connection_file.read(&length_, 4, 1);
		if (length_ < 0) {
			error("unkown file format!");
		}		
		return;
	}

	int MetaListObject::getLength() {
		return (length_);
	}

	void MetaListObject::setLength(int length) {
		length_ = length;
		return;
	}

	void MetaListObject::writeNameBit (ConnectionFile & connection_file) {
		connection_file.seekWrite(HAS_NAME_POSITION, SEEK_SET);
		connection_file.write(&has_name_, 1, 1);
		return;
	}

	void MetaListObject::readNameBit (ConnectionFile & connection_file) {
		connection_file.seekRead(HAS_NAME_POSITION, SEEK_SET);
		connection_file.read(&has_name_, 1, 1);
		return;
	}

	bool MetaListObject::getNameBit () {
		return (has_name_);
	}

	void MetaListObject::setNameBit (bool has_name) {
		has_name_ = has_name;
		return;
	}

	void MetaListObject::writeCompressBit (ConnectionFile & connection_file) {
		connection_file.seekWrite(IS_COMPRESS_POSITION, SEEK_SET);
		connection_file.write(&is_compress_, 1, 1);
		return;
	}

	void MetaListObject::readCompressBit (ConnectionFile & connection_file) {
		connection_file.seekRead(IS_COMPRESS_POSITION, SEEK_SET);
		connection_file.read(&is_compress_, 1, 1);
		return;
	}

	bool MetaListObject::getCompressBit () {
		return (is_compress_);
	}

	void MetaListObject::setCompressBit (bool is_compress) {
		is_compress_ = is_compress;
		return;
	}

	//write list head
	void MetaListObject::writeListHead (ConnectionFile & connection_file) {
		connection_file.seekWrite(LIST_HEAD_POSITION, SEEK_SET);
		char list_head[5] ="\x13\x00\x00\x00";
		connection_file.write(&list_head[0], sizeof(char), 4);
		return;
	}

	//------------------------------ ListObject ------------------------

	ListObject::ListObject () {
		length_ = 0;
		PROTECT_WITH_INDEX(r_list_ = Rf_allocVector(VECSXP, length_), &ipx);
		for (int i = 0; i < length_; i++) {
			SET_VECTOR_ELT(r_list_, i, R_NilValue);
		}
		names_.resize(length_);
		serialized_length_.resize(length_);
		has_name_ = false;
		is_compress_ = false;
	}

	ListObject::ListObject (int length, bool is_compress) {
		length_ = length;
		PROTECT_WITH_INDEX(r_list_ = Rf_allocVector(VECSXP, length_), &ipx);
		for (int i = 0; i < length_; i++) {
			SET_VECTOR_ELT(r_list_, i, R_NilValue);
		}
		names_.resize(length_);
		serialized_length_.resize(length_);
		has_name_ = false;
		is_compress_ = is_compress;
	}

	ListObject::ListObject (SEXP list, bool is_compress) {
		length_ = Rf_xlength(list);
		PROTECT_WITH_INDEX(r_list_ = list, &ipx);
		names_.resize(length_);
		serialized_length_.resize(length_);
		is_compress_ = is_compress;

		// get the character vector name_sxp from the names of list
		SEXP name_sxp = Rf_getAttrib(list, R_NamesSymbol);
		if (name_sxp == R_NilValue) {
			has_name_ = false;
			for (int i = 0; i < length_; i++) {
				names_[i].resize(NAMELENGTH);
				names_[i].assign(NAMELENGTH, '\xff');
			}
		} else {
			has_name_ = true;
			for (int i = 0; i < length_; i++) {
				names_[i] = UnitObject::charsxpToString(STRING_ELT(name_sxp, i));
			}
		}
	}

	ListObject::~ListObject() {
		UNPROTECT_PTR(r_list_);
	}

	// check if the objects are acceptable
	void ListObject::check() {
		for (int i = 0; i < length_; i++) {
			UnitObject unit_object(VECTOR_ELT(r_list_, i));
			unit_object.check();
		}
		return;
	}

	void ListObject::write(ConnectionFile & connection_file, MemorySlot & memory_slot, int index) {
		UnitObject unit_object(VECTOR_ELT(r_list_, index));
		serialized_length_[index] = unit_object.write(connection_file, memory_slot, is_compress_);
		return;
	}

	void ListObject::read(ConnectionFile & connection_file, MemorySlot & memory_slot, int index) {
		UnitObject unit_object;
		unit_object.read(connection_file, memory_slot, serialized_length_[index], is_compress_);
		SET_VECTOR_ELT(r_list_, index, unit_object.get());
		return;
	}

	void ListObject::resize(int length) {
		length_ = length;
		REPROTECT(r_list_ = Rf_lengthgets(r_list_, length_), ipx);
		names_.resize(length_);
		//Rprintf("length %d, original size %d\n", length_, serialized_length_.size());
		serialized_length_.resize(length_);
		return;
	}

	void ListObject::set (SEXP r_object, int index) {
		SET_VECTOR_ELT(r_list_, index, r_object);
		return;
	}

	void ListObject::setName(std::string name, int index) {
		names_[index] = name;
		return;
	}

	std::string ListObject::getName(int index) {
		return (names_[index]);
	}

	// turn the ListObject to a list object in R.
	SEXP ListObject::assembleRList () {
		// Rprintf("Length %d \n", length_);
		SEXP output_list = r_list_;
		SEXP names_sxp = PROTECT(Rf_allocVector(STRSXP, length_));
		std::string na_string(NAMELENGTH, '\xff');
		for (int i = 0; i < length_; i ++) {
			names_[i] == na_string ?
			SET_STRING_ELT(names_sxp, i, NA_STRING) :
			SET_STRING_ELT(names_sxp, i, Rf_mkChar(names_[i].c_str()));
		}
		if (has_name_ == true) {
			Rf_setAttrib(output_list, R_NamesSymbol, names_sxp);
		}
		UNPROTECT_PTR(names_sxp);
		return (output_list);
	}

	// get the serialized lengths of all objects in the list.
	void ListObject::calculateSerializedLength (MemorySlot & memoryslot) {
		large_list::ProgressReporter calculate_reporter;
		for (int i = 0; i < length_; i ++) {
			UnitObject unit_object(VECTOR_ELT(r_list_, i));
			serialized_length_[i] = unit_object.calculateSerializedLength(memoryslot, is_compress_);
			// Rprintf("LENGTH %3.0ld \n", serialized_length_[i]);
			// Print progress to console
			calculate_reporter.reportProgress(i, length_, "Calculate Serialized Length");
		}
		return;
	}

	void ListObject::setSerializedLength(int64_t serialized_length, int index) {
		serialized_length_[index] = serialized_length;
	}

	int64_t ListObject::getSerializedLength(int index) {
		return serialized_length_[index];
	}

	void ListObject::print() {
		Rprintf("Length %d, Has_name %s, Is_compress %s \n",
				length_,
				has_name_ ? "true" : "false",
				is_compress_ ? "true" : "false");
		for (int i = 0; i < length_; i++) {
			Rprintf("index %d, serialized_length_ %lf, name %s \n",
					 i,
					(double) serialized_length_[i],
					names_[i].c_str());
		}
	}
}
