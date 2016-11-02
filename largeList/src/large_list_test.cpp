#include "large_list.h"

extern "C" SEXP largeListTest() {
  int a[2] = {1, 0};
  int index = 2;
  int b=a[index];
  Rprintf("wrong: %d", b);
	// class vector_class {
	// private:
	// 	PROTECT_INDEX ipx;
	// 	SEXP r_list_;
	// 	std::vector<std::string> names_;
	// 	std::vector<int64_t> serialized_length_;
	// 	bool has_name_;
	// 	bool is_compress_;
	// 	int length_;
	// public:
	// 	vector_class(int length, bool is_compress) {
	// 		length_ = length;
	// 		PROTECT_WITH_INDEX(r_list_=Rf_allocVector(VECSXP, length_), &ipx);
	// 		for (int i = 0; i < length_; i++) {
	// 			SET_VECTOR_ELT(r_list_, i, R_NilValue);
	// 		}
	// 		names_.resize(length_, "");
	// 		serialized_length_.resize(length_);
	// 		has_name_ = false;
	// 		is_compress_ = is_compress;
	// 		return;
	// 	}
	// 	~vector_class() {
	// 		UNPROTECT_PTR(r_list_);
	// 	}
	// 	void resize(int length) {
	// 		int old_length = Rf_length(r_list_);
	// 		length_ = length;
	// 		REPROTECT(r_list_ = Rf_lengthgets(r_list_, length_), ipx);
	// 		for (int i = old_length; i < length_; i++) {
	// 			SET_VECTOR_ELT(r_list_, i, R_NilValue);
	// 		}
	// 		names_.resize(length_, "");
	// 		//printf("length %d, original size %d\n", length_, serialized_length_.size());
	// 		serialized_length_.resize(length_, 0);
	// 		return;
	// 	}
	// };
	// vector_class vec(0, false);
	// for (int i = 0; i< 10000; i++) {
	// 	vec.resize(0);
	// 	vec.resize(i);
	// 	//cout << "mem_add" << &vec << endl;
	// 	//cout <<  "length" << vec.size()<< endl;
	// }
	/*SEXP r_list_;
	int length_ = 100;
	PROTECT_INDEX ipx;
	PROTECT_WITH_INDEX(r_list_=allocVector(VECSXP, length_), &ipx);
			for (int i = 0; i < length_; i++) {
				SET_VECTOR_ELT(r_list_, i, ScalarInteger(i));
			}

	REPROTECT(r_list_ = Rf_lengthgets(r_list_, length_*2), ipx);
				for (int i = length_; i < 2*length_; i++) {
				SET_VECTOR_ELT(r_list_, i, ScalarInteger(i));
			}

	Rprintf("new length %d \n", Rf_length(r_list_));
	for (int i = 0; i < 2*length_; i++) {
		 Rprintf("%d ", INTEGER(VECTOR_ELT(r_list_, i))[0]);
	}
	UNPROTECT_PTR(r_list_);*/
	return(ScalarLogical(1));
}