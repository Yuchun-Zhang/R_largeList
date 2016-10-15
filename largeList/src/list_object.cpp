#include "large_list.h"
namespace large_list {
	//-------------------------- MetaListObject ------------------------
	MetaListObject::MetaListObject (){
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
		connection_file.write((char *) & (length_), 4, 1);
		return;
	}

	void MetaListObject::readLength (ConnectionFile & connection_file) {
		connection_file.seekRead(LENGTH_POSITION, SEEK_SET);
		connection_file.read((char *) & (length_), 4, 1);
		return;
	}

	int MetaListObject::getLength() {
		return(length_);
	}

	void MetaListObject::setLength(int length) {
		length_ = length;
		return;
	}

	void MetaListObject::writeNameBit (ConnectionFile & connection_file) {
		connection_file.seekWrite(HAS_NAME_POSITION, SEEK_SET);
		connection_file.write((char *) &(has_name_), 1, 1);
		return;
	}

	void MetaListObject::readNameBit (ConnectionFile & connection_file) {
		connection_file.seekRead(HAS_NAME_POSITION, SEEK_SET);
		connection_file.read((char *) &(has_name_), 1, 1);
		return;
	}

	bool MetaListObject::getNameBit () {
		return(has_name_);
	}

	void MetaListObject::setNameBit (bool has_name) {
		has_name_ = has_name;
		return;
	}

	//write list head
	void MetaListObject::writeListHead (ConnectionFile & connection_file){
		connection_file.seekWrite(LIST_HEAD_POSITION, SEEK_SET);
		std::string list_head("\x13\x00\x00\x00");
    	connection_file.write((char *)list_head.c_str(), 1, 4);
    	return;
	}

	//------------------------------ ListObject ------------------------

	ListObject::ListObject (){}

	ListObject::ListObject (int length){
		length_ = length;
		list_.resize(length_);
		names_.resize(length_, "");
		serialized_length_.resize(length_, 0);
		has_name_ = false;
		return;
	}

	ListObject::ListObject (SEXP list){
		length_ = Rf_xlength(list);
		list_.resize(length_);
		names_.resize(length_, "");
		serialized_length_.resize(length_, 0);

		// get the character vector name_sxp from the names of list
		SEXP name_sxp = Rf_getAttrib(list, R_NamesSymbol);
		if (name_sxp == R_NilValue) {
			has_name_ = false;
			name_sxp = PROTECT(Rf_allocVector(STRSXP, Rf_length(list)));
			for (int i = 0 ; i < Rf_length(list); i ++) {
				SET_STRING_ELT(name_sxp, i, NA_STRING);
			}
			UNPROTECT(1);
		} else {
			has_name_ = true;
		}

		// assgin values
		for (int i = 0; i < length_; i++) {
			list_[i].set(VECTOR_ELT(list, i));
      		names_[i] = UnitObject::charsxpToString(STRING_ELT(name_sxp, i));
      	}
      	return;
	}

	ListObject::~ListObject() {}

	// check if the objects are acceptable
	void ListObject::check() {
		for (int i = 0; i < length_; i++) {
			list_[i].check();
		}
		return;
	}

	void ListObject::write(ConnectionFile & connection_file, int index) {
		list_[index].write(connection_file);
		return;
	}

	void ListObject::read(ConnectionFile & connection_file, int index) {
		list_[index].read(connection_file);
		return;
	}

	void ListObject::resize(int length) {
		length_ = length;
		list_.resize(length_);
		names_.resize(length_, "");
		serialized_length_.resize(length_, 0);
		return;
	}

	void ListObject::set (SEXP r_object, int index) {
		list_[index].set(r_object);
		return;
	}

	void ListObject::setName(std::string name, int index) {
		names_[index] = name;
		return;
	}

	std::string ListObject::getName(int index){
		return(names_[index]);
	};

	// turn the ListObject to a list object in R.
	SEXP ListObject::assembleRList (ConnectionFile & connection_file) {
		// Rprintf("Length %d \n", length_);
		SEXP output_list = PROTECT(Rf_allocVector(VECSXP, length_));
		SEXP names_sxp = PROTECT(Rf_allocVector(STRSXP, length_));
		std::string na_string(NAMELENGTH, '\xff');
		for (int i = 0; i < length_; i ++) {
			SEXP temp = PROTECT(list_[i].get());
			//Rprintf("Number %d, Type %d \n", i, TYPEOF(temp));
			SET_VECTOR_ELT(output_list, i, temp);
			UNPROTECT_PTR(temp);
			//SET_VECTOR_ELT(output_list, i, R_NilValue);
			//Rprintf("%s \n", names_[i].c_str());
			names_[i] == na_string ?
      			SET_STRING_ELT(names_sxp, i, NA_STRING) :
      			SET_STRING_ELT(names_sxp, i, Rf_mkChar(names_[i].c_str()));
		}
		readNameBit(connection_file);
		if (has_name_ == true) {
			Rf_setAttrib(output_list, R_NamesSymbol, names_sxp);
		}
      	UNPROTECT_PTR(names_sxp);
      	UNPROTECT_PTR(output_list);
      	return(output_list);
	}

	// get the serialized lengths of all objects in the list.
	void ListObject::calculateSerializedLength () {
		for (int i = 0; i < length_; i ++){
			serialized_length_[i] = list_[i].calculateSerializedLength();
			// Rprintf("LENGTH %3.0ld \n", serialized_length_[i]);
		}
		return;
	}

	int64_t ListObject::getSerializedLength(int index) {
		return serialized_length_[index];
	}
}
