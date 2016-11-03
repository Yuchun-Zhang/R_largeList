#include "large_list.h"
namespace large_list {
	// the constructor transfers the numeric/character R vetor to an integer vector index_.
	IndexObject::IndexObject(SEXP index, 
							 int list_length, 
							 ConnectionFile & connection_file, 
							 bool extend_to_list_length) {
		list_length_ = list_length;
		if (TYPEOF(index) == NILSXP) {
			length_ = list_length;
			index_.resize(length_);
			for (int i = 0; i < length_; i ++) {
				index_[i] = i;
			}
		}
		if (TYPEOF(index) == LGLSXP) {
			length_ = extend_to_list_length ? list_length : Rf_length(index);
			//Rprintf("length_ : %d \n", length_);
			index_.resize(length_);
			int value_length = Rf_length(index);
			int count  = 0;
			for (int i = 0; i < length_; i ++) {
				if (LOGICAL(index)[i % value_length] == R_NaInt) {index_[count] = R_NaInt; count ++; }
				if (LOGICAL(index)[i % value_length] == 1) {index_[count] = i; count ++;}
			}
			length_ = count;
			index_.resize(length_);
		}
		if (TYPEOF(index) == INTSXP ||  TYPEOF(index) == REALSXP) {
			length_ = Rf_length(index);
  			index_.resize(length_);
  			TYPEOF(index) == INTSXP ?
  				index_.assign(INTEGER(index), INTEGER(index) + length_) :
  				index_.assign(REAL(index), REAL(index) + length_);
  			try {processNumeric(); } catch (std::exception &e){ connection_file.disconnect(); error(e.what());}
  			// Rprintf("index size : %d, length_ : %d\n", index_.size(), length_);
		}
		if (TYPEOF(index) == STRSXP) {
			length_ = Rf_length(index);
			index_.resize(length_);
			int64_t temp_position;
			int temp_index;
			std::string temp_name_string;
			for (int i = 0; i < length_; i ++) {
				if (STRING_ELT(index, i) == NA_STRING) {
					index_[i] = R_NaInt;
				} else {
					temp_name_string = UnitObject::charsxpToString(STRING_ELT(index, i));
					fileBinarySearchByName(connection_file, temp_position, temp_name_string, temp_index, list_length_);
					fileBinarySearchByPosition(connection_file, temp_position, index_[i], list_length_);
					// Rprintf("Index is %d \n", index_[i]);
				}
			}
		}
		tuple_object_.resize(length_);
	}

	IndexObject::~IndexObject(){}

	// search given name in the position-name pairs in the file.
	void IndexObject::fileBinarySearchByName(ConnectionFile & connection_file, 
											 int64_t & position, 
											 std::string name, 
											 int & index, 
											 int length){
		int left = 0;
		int right = length - 1;
		int mid;
		std::string current_name(NAMELENGTH, '\xff');
		while (left <= right) {
			mid =  (left + right) / 2;
			connection_file.seekRead(-(8 + NAMELENGTH) * length + mid * (8 + NAMELENGTH) + 8, SEEK_END);
			connection_file.read((char*) & (current_name[0]), NAMELENGTH , 1);
			// Rprintf("%d \n", current_name.size());
			// Rprintf("%d \n", name.size());
			// Rprintf("mid is %d \n", mid);
			// Rprintf("current name %s \n", current_name.c_str());
			// Rprintf("name %s \n", name.c_str());
			if (current_name == name) {
				index = mid;
				connection_file.seekRead(-(8 + NAMELENGTH) * length + mid * (8 + NAMELENGTH), SEEK_END);
				connection_file.read((char*) & (position), 8, 1);
				return;
			} else if (current_name > name) {
				right = mid - 1;
			} else {
				left = mid + 1;
			}
		}
		index = R_NaInt;
		position = -1;
		return;
	}

	//given position, find the corresponding index.
	void IndexObject::fileBinarySearchByPosition (ConnectionFile & connection_file,
												  int64_t & position, 
												  int & index, 
												  int & length) {
		int left = 0;
		int right = length - 1;
		int mid;
		int64_t current_position;
		while (left <= right) {
			mid = (left + right) / 2;
			connection_file.seekRead(-2 * (8 + NAMELENGTH) * length - 8 + mid * (8 + NAMELENGTH), SEEK_END);
			connection_file.read((char*) & (current_position), 8, 1);
			if (current_position == position) {
				index = mid;
				return;
			} else if (current_position > position) {
				right = mid - 1;
			} else {
				left = mid + 1;
			}
		}
		index = R_NaInt;
		return;
	}

	// deal with the given numeric index, do some preprocess.
	void IndexObject::processNumeric() {
		bool has_positive = false;
		bool has_negative = false;
		bool has_zero = false;
		bool has_na = false;
		for (int i = 0 ; i < length_; i ++) {
			if (index_[i] > 0) { has_positive = true; }
			if (index_[i] < 0 && index_[i] != R_NaInt) { has_negative = true; }
			if (index_[i] == R_NaInt) {has_na = true; }
			if (index_[i] == 0)  {has_zero = true;}
		}
		if ((has_positive == true && has_negative == true) || (has_na == true && has_negative == true)) {
			throw std::runtime_error("only 0's may be mixed with negative subscripts");
		}

		if (has_zero == true) {
			index_.erase( std::remove(index_.begin(), index_.end(), 0), index_.end() );
			/*int count  = 0;
			for (int i = 0; i < length_; i ++) {
				if (index_[i] != 0) {index_[count] = index_[i]; count ++; }
			}
			length_ = count;
			index_.resize(length_);*/
			length_ = index_.size();
		}

		if (has_positive == false && has_negative == true) {
			for (int i = 0 ; i < length_; i++) {
				index_[i] = - index_[i];
			}
			std::sort(index_.begin(), index_.end());
			std::vector<int> res_num(list_length_);
			int ptr_to_index_num = 0;
			int current_number = 1;
			int ptr_to_res_num = 0;
			while (current_number <= list_length_) {
				//Rprintf("%d \n", current_number);
				if (ptr_to_index_num == length_) {
					res_num[ptr_to_res_num] = current_number;
					current_number ++;
					ptr_to_res_num ++;
					continue;
				}
				if (current_number < index_[ptr_to_index_num]) {
					res_num[ptr_to_res_num] = current_number;
					current_number ++;
					ptr_to_res_num ++;
					continue;
				}
				if (current_number > index_[ptr_to_index_num]) {
					while (current_number > index_[ptr_to_index_num] && ptr_to_index_num < length_) {
						ptr_to_index_num ++;
					}
					continue;
				}
				if (current_number == index_[ptr_to_index_num]) {
					current_number ++;
					if (ptr_to_index_num < length_) { ptr_to_index_num ++; }
					continue;
				}
			}
			res_num.resize(ptr_to_res_num);
			index_.assign(res_num.begin(), res_num.end());
			length_ = index_.size();
		}
		// change 1..N to 0..N-1
		for (int i = 0; i < length_; i++) {
			if (index_[i] != R_NaInt) {
				index_[i] = index_[i] - 1;
			}
		}
		// mark numbers > list_length_
		for (int i = 0; i < length_; i++) {
			if (index_[i] >= list_length_) {
				index_[i] = R_NaInt;
			}
		}
		return;
	}

	void IndexObject::removeInvalid () {
		//remove invalide elements in the delete list
		int temp_index = 0;
		for (int i = 0 ; i < length_ ; i++) {
			if (index_[i] !=  R_NaInt) {
				index_[temp_index] = index_[i];
				temp_index ++ ;
			}
		}
		index_.resize(temp_index);
		length_ = temp_index;
	}

	// read the position-name pairs from the file.
	void IndexObject::readPair(ConnectionFile & connection_file) {
		for (int i = 0 ; i < length_; i++) {
			// Rprintf("Index is %d \n", index_[i]);
			if (index_[i] != R_NaInt) {
				//Rprintf("real %lf \n", (double)(int64_t)(-2 * (8 + NAMELENGTH) * list_length_ - 8 + index_[i] * (8 + NAMELENGTH)));
				connection_file.seekRead(-2 * (8 + NAMELENGTH) * list_length_ - 8 + index_[i] * (8 + NAMELENGTH), SEEK_END);
				tuple_object_.read(connection_file, i);
			} else {
				tuple_object_.setToInvalid(i);
			}
		}
	}

	int64_t IndexObject::getPosition(int index) {
		return tuple_object_.getPosition(index);
	}

	int64_t IndexObject::getSerializedLength(int index) {
		return tuple_object_.getSerializedLength(index);
	}

	std::string IndexObject::getName(int index) {
		return tuple_object_.getName(index);
	}

	int IndexObject::getLength(){
		return length_;
	}

	int IndexObject::getIndex(int index){
		return index_[index];
	}

	void IndexObject::sort() {
		std::sort(index_.begin(), index_.end());
		return;
	}

	void IndexObject::removeDuplicate() {
		index_.erase( std::unique( index_.begin(), index_.end() ), index_.end() );
		length_ = index_.size();	
		return;
	}

	void IndexObject::print (int type) {
		Rprintf("index contents : \n");
		for (int i = 0; i < length_; i ++) {
			Rprintf("Index %d  \n", index_[i]);
			if (type > 1) {tuple_object_.print(i);}
		}
	}

	// ------------------- IndexWithValueObject --------------------
	IndexWithValueObject::IndexWithValueObject (SEXP index, 
											    int list_length, 
											    ConnectionFile &connection_file, 
											    bool extend_to_list_length):
		IndexObject(index, list_length, connection_file, extend_to_list_length){}

	IndexWithValueObject::~IndexWithValueObject (){}

	void IndexWithValueObject::setValueLength(int value_length) {
		value_length_ = value_length;
		return;
	}

	void IndexWithValueObject::setValueIndex() {
	  index_pair_.resize(length_);
		for (int i = 0; i < length_; i ++) { 
			index_pair_[i].first = index_[i];
			index_pair_[i].second = i;
		}
		return;
	}

	bool IndexWithValueObject::cmp (std::pair<int, int> const & a, std::pair<int, int> const & b){
		return a.first != b.first ?  a.first < b.first : a.second > b.second;
	}

	void IndexWithValueObject::removeInvalid () {
		//remove invalide elements in the delete list
		int temp_index = 0;
		for (int i = 0 ; i < length_ ; i++) {
			if (index_pair_[i].first !=  R_NaInt) {
				index_pair_[temp_index] = index_pair_[i];
				temp_index ++ ;
			}
		}
		index_pair_.resize(temp_index);
		length_ = temp_index;
	}

	void IndexWithValueObject::sort() {
		std::sort(index_pair_.begin(), index_pair_.end(), cmp);
		return;
	}

	void IndexWithValueObject::removeDuplicate() {
		if (length_ >= 2) {
			for (int i = 1; i < length_; i ++) {
				if (index_pair_[i].first == index_pair_[i - 1].first) {
					index_pair_[i].second = index_pair_[i - 1].second;
				}
			}
		}
		index_pair_.erase( std::unique( index_pair_.begin(), index_pair_.end() ), index_pair_.end() );
		length_ = index_pair_.size();
		index_.resize(length_);
		for (int i = 0; i < length_; i ++) {
			index_[i] = index_pair_[i].first;
		}
		return;
	}

	int IndexWithValueObject::getValueIndex(int index) {
		return index_pair_[index].second % value_length_;
	}

	int IndexWithValueObject::getIndex(int index) {
		return index_pair_[index].first;
	}

	void IndexWithValueObject::print () {
		Rprintf("index contents : \n");
		for (int i = 0; i < length_; i ++) {
			Rprintf("Index %d  \n", index_pair_[i].first);
			Rprintf("Value Index %d  \n", index_pair_[i].second);
		}
	}
}