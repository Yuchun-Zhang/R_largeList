#include "large_list.h"
namespace large_list {

	UnitObject::UnitObject () {
		// Rprintf("Unit Object Initial Default \n");
		PROTECT(r_object_ = R_NilValue);
	}

	UnitObject::~UnitObject() {
		// Rprintf("Unit Object Destruction \n");
		UNPROTECT_PTR(r_object_);
	}

	UnitObject::UnitObject (SEXP r_object) {
		// Rprintf("Unit Object Initial Given Value \n");
		PROTECT(r_object_ = r_object);
	}

	void UnitObject::set (SEXP r_object) {
		// Rprintf("Unit Object Set Value \n");
		UNPROTECT_PTR(r_object_);
		PROTECT(r_object_ = r_object);
	}

	SEXP UnitObject::get () {
		return (r_object_);
	}

	void UnitObject::check () {
		checkSEXP(r_object_);
	}

	int64_t UnitObject::write (ConnectionFile & connection_file, bool is_compress) {
		// direct write via ConnectionFile is deprecated as it's slower in most cases.
		// writeSEXP(r_object_, connection_file);
		// return(calculateSerializedLength(false));

		// write via ConnectionRaw
		int64_t uncompressed_ser_len = calculateSerializedLength(false);
		ConnectionRaw connection_raw(uncompressed_ser_len);
		writeSEXP(r_object_, connection_raw);
		if (is_compress) {connection_raw.compress();}
		connection_file.write(connection_raw.getRaw(), connection_raw.getLength(), 1);
		return (connection_raw.getLength());
	}

	void UnitObject::read (ConnectionFile & connection_file, int64_t serialized_length, bool is_compress) {
		// direct read via ConnectionFile is deprecated
		// UNPROTECT_PTR(r_object_);
		// PROTECT(r_object_ = readSEXP(connection_file));

		// read via ConnectionRaw
		ConnectionRaw connection_raw(serialized_length);
		connection_file.read(connection_raw.getRaw(), connection_raw.getLength(), 1);
		if (is_compress) {connection_raw.uncompress();}
		UNPROTECT_PTR(r_object_);
		PROTECT(r_object_ = readSEXP(connection_raw));
	}

	int64_t UnitObject::calculateSerializedLength(bool is_compress) {
		if (is_compress) {
			int64_t uncompressed_ser_len = calculateSerializedLength(false);
			ConnectionRaw connection_raw(uncompressed_ser_len);
			writeSEXP(r_object_, connection_raw);
			connection_raw.compress();
			return connection_raw.getLength();
		} else {
			int64_t length = 0;
			lengthOfSEXP(r_object_, length);
			return length;
		}
	}

	// --------------------------- SEXP functions -------------------------------------

	//get the info of the object, e.g. level, is a object?, attribute, tag.
	void UnitObject::getHeadInfo(SEXP _x, int &level, int &object, SEXP & attribute, SEXP & tag) {
		attribute = TYPEOF(_x) == CHARSXP ? R_NilValue : ATTRIB(_x);
		level = TYPEOF(_x) == CHARSXP ? LEVELS(_x) & 65502 : LEVELS(_x);
		tag = TYPEOF(_x) == LISTSXP ? TAG(_x) : R_NilValue;
		object = OBJECT(_x);
		return;
	}

	void UnitObject::writeLength(SEXP _x, Connection & connection) {
		int length = LENGTH(_x);
		connection.write((char *)&length, 4, 1);
		return;
	}

	//read object length
	void UnitObject::readLength(Connection & connection, int &length) {
		connection.read((char*)(&length), 4, 1);
		return;
	}

	void UnitObject::writeSEXP(SEXP _x, Connection & connection) {
		int level, object;
		SEXP attribute, tag;
		getHeadInfo(_x, level, object, attribute, tag);
		if (TYPEOF(_x) != NILSXP) {
			//write head
			int head = TYPEOF(_x) +
			(object << 8) +
			((int)(attribute != R_NilValue) << 9) +
			((int)(tag != R_NilValue) << 10) +
			(level << 12);
			connection.write((char *)(&head), 1, 4);
		}
		switch (TYPEOF(_x)) {
		case NILSXP   : { 			
			BYTE nil[4] = {0xfe, 0x00, 0x00, 0x00};
			connection.write((char *) & (nil[0]), 1, 4);  
			break; 
		}
		case REALSXP  : {
			writeLength(_x, connection);
			connection.write((char *)(& REAL(_x)[0]), 8, Rf_xlength(_x));
			break;
		}
		case INTSXP   : { 
			writeLength(_x, connection);
			connection.write((char *)(& INTEGER(_x)[0]), 4, Rf_xlength(_x));
		  	break; 
		 }
		case CPLXSXP  : { 
			writeLength(_x, connection);
			connection.write((char *)(& COMPLEX(_x)[0]), 16, Rf_xlength(_x));
			break; 
		}
		case CHARSXP   : { 
			if (_x == NA_STRING ) {
				BYTE ffff[4] = {0xff, 0xff, 0xff, 0xff};
				connection.write((char *) & (ffff[0]), 1, 4);
			} else {
				writeLength(_x, connection);
				connection.write((char*)CHAR(_x), 1, Rf_xlength(_x));
			}
		    break; 
		}
		case STRSXP : {
			writeLength(_x, connection);
			for (int64_t i = 0; i < Rf_xlength(_x); i++) {
				writeSEXP(STRING_ELT(_x, i), connection);
			}
		 	break; 
		}
		case LGLSXP   : { 
			writeLength(_x, connection);
			connection.write((char *)(& LOGICAL(_x)[0]), 4, Rf_xlength(_x));
		  break; 
		}
		case VECSXP   : { 
			writeLength(_x, connection);
			for (int64_t i = 0; i < Rf_xlength(_x); i++) {
				writeSEXP(VECTOR_ELT(_x, i), connection);
			}
		  	break; 
		}
		case RAWSXP   : { 
			writeLength(_x, connection);
			connection.write((char *)(& RAW(_x)[0]), 1, Rf_xlength(_x));
		  	break; 
		}
		case SYMSXP   : { 
			SEXP name = PRINTNAME(_x);
			writeSEXP(name, connection);
		  	break; 
		}
		case LISTSXP  : {
			SEXP el = CAR(_x);
			SEXP tag = TAG(_x);
			writeSEXP(tag, connection);
			writeSEXP(el, connection);
			break;
		}
		default : {throw std::runtime_error("Data type not supported. Please check ?largeList");}
		}
		if (attribute != R_NilValue) {
			for (SEXP nxt = attribute; nxt != R_NilValue; nxt = CDR(nxt)) {
				writeSEXP(nxt, connection);
			}
			BYTE nil[4] = {0xfe, 0x00, 0x00, 0x00};
			connection.write((char *) & (nil[0]), 1, 4);  
		}
		return ;
	}

	SEXP UnitObject::readSEXP(Connection & connection) {
		int type, has_attr, has_tag, has_object, level, head_info;
		connection.read((char*)(&head_info), 4 , 1);
		type = head_info & 255;
		has_object = (head_info >> 8) & 1;
		has_attr = (head_info >> 9) & 1;
		has_tag =  (head_info >> 10) & 1;
		level = head_info >> 12;

		SEXP element = R_NilValue;
		int length;
		switch (type) {
		case 0xfe : element = PROTECT(R_NilValue); break;
		case REALSXP  : {
			readLength(connection, length);
			element = PROTECT(Rf_allocVector(REALSXP, length));
			connection.read((char*)(&REAL(element)[0]), 8, length);
			break;
		}
		case INTSXP   : {
			readLength(connection, length);
			element = PROTECT(Rf_allocVector(INTSXP, length));
			connection.read((char*)(&INTEGER(element)[0]), 4, length);
			break;
		}
		case CPLXSXP  : { 
			readLength(connection, length);
			element = PROTECT(Rf_allocVector(CPLXSXP, length));
			connection.read((char*)(&COMPLEX(element)[0]), 16, length);
			break;
		}
		case STRSXP   : {
			readLength(connection, length);
			element = PROTECT(Rf_allocVector(STRSXP, length));
			for (int64_t i = 0; i < length; i++) {
				SEXP temp = PROTECT(readSEXP(connection));
				SET_STRING_ELT(element, i, temp);
				UNPROTECT(1);
			}
			break;
		}
		case CHARSXP  : { 
			std::vector<BYTE> tempRaw(4);
			std::vector<BYTE> naString(4, 0xff);
			connection.read((char *) & (tempRaw[0]), 1, 4);
			if (tempRaw == naString) {
				element = PROTECT(NA_STRING);
			} else {
				std::string x;
				connection.seekRead(-4, SEEK_CUR);
				connection.read((char *) & (length), 4, 1);
				x.resize(length);
				connection.read((char*)(&x[0]), 1, length);
				element = PROTECT(Rf_mkChar(x.c_str()));
			}
			break;
		}
		case LGLSXP   : {
			readLength(connection, length);
			element = PROTECT(Rf_allocVector(LGLSXP, length));
			connection.read((char*)(&LOGICAL(element)[0]), 4, length);
			break;
		}
		case VECSXP   : {
			readLength(connection, length);
			element = PROTECT(Rf_allocVector(VECSXP, length));
			for (int64_t i = 0; i < length; i++) {
				SEXP temp = PROTECT(readSEXP(connection));
				SET_VECTOR_ELT(element, i, temp);
				UNPROTECT(1);
			}
			break;
		}
		case RAWSXP   : { 
			readLength(connection, length);
			element = PROTECT(Rf_allocVector(RAWSXP, length));
			connection.read((char*)(&RAW(element)[0]), 1, length);
			break;
		}
		case SYMSXP   : {
			SEXP chars = PROTECT(readSEXP(connection));
			element = PROTECT(Rf_install(CHAR(chars)));
			UNPROTECT_PTR(chars);
			break;
		}
		case LISTSXP  : {
			SEXP tag;
			SEXP el;
			if (has_tag == 1) {
				tag = PROTECT(readSEXP(connection));
			}
			el = PROTECT(readSEXP(connection));
			element = PROTECT(Rf_cons(el, R_NilValue));
			UNPROTECT_PTR(el);
			if (has_tag == 1) {SET_TAG(element, tag); UNPROTECT_PTR(tag);}
			break;
		} 
		}
		if (has_attr == 1) {
			SEXP parlist = PROTECT(readSEXP(connection));
			SEXP current_chain = parlist;
			while (true) {
				SEXP chain = PROTECT(readSEXP(connection));
				if (chain != R_NilValue) {
					current_chain = SETCDR(current_chain, chain);
					UNPROTECT_PTR(chain);
				} else {
					UNPROTECT_PTR(chain);
					break;
				}
			}
			SET_ATTRIB(element, parlist);
			UNPROTECT_PTR(parlist);
		}
		if (has_object == 1) {
			SET_OBJECT(element, 1);
		}
		SETLEVELS(element, level);
		UNPROTECT(1);
		return element;
	}

	void UnitObject::checkSEXP(SEXP _x) {
		int level, object;
		SEXP attribute, tag;
		getHeadInfo(_x, level, object, attribute, tag);
		switch (TYPEOF(_x)) {
		case NILSXP   : { break; }
		case REALSXP  : { break; }
		case INTSXP   : { break; }
		case CPLXSXP  : { break; }
		case STRSXP   : { break; }
		case CHARSXP  : { break; }
		case LGLSXP   : { break; }
		case VECSXP   : {
			for (int64_t i = 0; i < Rf_xlength(_x); i++) {
				checkSEXP(VECTOR_ELT(_x, i));
			}
			break;
		}
		case RAWSXP   : { break; }
		case SYMSXP   : {
			SEXP name = PRINTNAME(_x);
			checkSEXP(name);
			break;
		}
		case LISTSXP  : {
			SEXP el = CAR(_x);
			SEXP tag = TAG(_x);
			checkSEXP(tag);
			checkSEXP(el);
			break;
		}
		default : {
			throw std::runtime_error("Data type not supported. Please check ?largeList");
		}
		}
		if (attribute != R_NilValue) {
			for (SEXP nxt = attribute; nxt != R_NilValue; nxt = CDR(nxt)) {
				checkSEXP(nxt);
			}
		}
		return ;
	}

	std::string UnitObject::charsxpToString(SEXP char_sexp) {
		std::string str;
		if (char_sexp == NA_STRING) {
			str = std::string(NAMELENGTH, '\xff');
		} else {
			str = std::string(NAMELENGTH, '\x00');
			str.replace(0, Rf_length(char_sexp), CHAR(char_sexp));
		}
		return (str);
	}

	void UnitObject::lengthOfSEXP(SEXP _x, int64_t &length) {
		int level, object;
		SEXP attribute, tag;
		getHeadInfo(_x, level, object, attribute, tag);
		if (TYPEOF(_x) != NILSXP) {
			length += 4;
		}
		switch (TYPEOF(_x)) {
		case NILSXP   : { length += 4;  break; }
		case REALSXP  : { length += 8 * Rf_xlength(_x) + 4; break; }
		case INTSXP   : { length += 4 * Rf_xlength(_x) + 4;  break; }
		case CPLXSXP  : { length += 16 * Rf_xlength(_x) + 4; break; }
		case STRSXP   : {
			length += 4;
			for (int64_t i = 0; i < Rf_xlength(_x); i++) {
				lengthOfSEXP(STRING_ELT(_x, i), length);
			}
			break;
		}
		case CHARSXP  : { _x == NA_STRING ?  length += 4 : length += 1 * Rf_xlength(_x) + 4; break; }
		case LGLSXP   : { length += 4 * Rf_xlength(_x) + 4;  break; }
		case VECSXP   : {
			length += 4;
			for (int64_t i = 0; i < Rf_xlength(_x); i++) {
				lengthOfSEXP(VECTOR_ELT(_x, i), length);
			}
			break;
		}
		case RAWSXP   : { length += 1 * Rf_xlength(_x) + 4;  break; }
		case SYMSXP   : {
			SEXP name = PRINTNAME(_x);
			lengthOfSEXP(name, length);
			break;
		}
		case LISTSXP  : {
			SEXP el = CAR(_x);
			SEXP tag = TAG(_x);
			lengthOfSEXP(tag, length);
			lengthOfSEXP(el, length);
			break;
		}
		default : {throw - 1;}
		}
		if (attribute != R_NilValue) {
			for (SEXP nxt = attribute; nxt != R_NilValue; nxt = CDR(nxt)) {
				lengthOfSEXP(nxt, length);
			}
			length += 4;
		}
		return ;
	}
}
