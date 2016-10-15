#include "large_list.h"
namespace large_list {
	NamePositionPair::NamePositionPair(){}
	NamePositionPair::NamePositionPair(int length){
		length_ = length;
		pair_.resize(length_, std::pair<int64_t, std::string> (0, ""));
		last_position_ = 0;
	}

	// initialize by deep copy
	NamePositionPair::NamePositionPair(NamePositionPair &toCopyObject) {
		length_ = toCopyObject.getLength();
		pair_.resize(length_, std::pair<int64_t, std::string> (0, ""));
		for (int i = 0; i < length_; i++) {
			pair_[i].first = toCopyObject.getPosition(i);
			pair_[i].second = toCopyObject.getName(i);
		}
		last_position_ = toCopyObject.getLastPosition();
	}

	NamePositionPair::~NamePositionPair(){}

	void NamePositionPair::resize(int length) {
		length_ = length;
		pair_.resize(length_, std::pair<int64_t, std::string> (0, ""));
	}

	// write the whole table 
	void NamePositionPair::write(ConnectionFile & connection_file, bool write_last_position){
		for (int i = 0; i < length_; i++) {
    		connection_file.write((char *) & (pair_[i].first), 8, 1);
    		connection_file.write((char*)pair_[i].second.c_str(), NAMELENGTH, 1);
		}
		if (write_last_position) {
			connection_file.write((char *) & (last_position_), 8, 1);
		}
  		return;
	}

	// read the whole table 
	void NamePositionPair::read(ConnectionFile & connection_file) {
		connection_file.seekRead(-(8 + NAMELENGTH) * length_ * 2 - 8, SEEK_END);
  		for (int i = 0; i < length_; i++) {
    		connection_file.read((char *) & (pair_[i].first), 8, 1);
    		pair_[i].second.resize(NAMELENGTH);
    		connection_file.read((char *) & ((pair_[i].second)[0]), NAMELENGTH, 1);
  		}
  		return;
	}

	// read one element
	void NamePositionPair::read(ConnectionFile & connection_file, int index) {
		connection_file.read((char *) & (pair_[index].first), 8, 1);
		pair_[index].second.resize(NAMELENGTH);
		connection_file.read((char *) & ((pair_[index].second)[0]), NAMELENGTH, 1);
  		return;
	}

	// sort first by name than by position
	void NamePositionPair::sort() {
		std::stable_sort(pair_.begin(), pair_.end(), cmp);
		return;
	}

	void NamePositionPair::setPosition(ConnectionFile & connection_file, int i) {
		pair_[i].first = connection_file.tellWrite();
		return;
	}

	void NamePositionPair::setPosition(int64_t position, int i) {
		pair_[i].first = position;
		return;
	}

	void NamePositionPair::setLastPosition(ConnectionFile & connection_file) {
		last_position_ = connection_file.tellWrite();
		return;
	}	

	void NamePositionPair::setLastPosition(int64_t last_position) {
		last_position_ = last_position;
		return;
	}	

	void NamePositionPair::readLastPosition(ConnectionFile & connection_file) {
		connection_file.seekRead(-(8 + NAMELENGTH) * length_ - 8, SEEK_END);
		connection_file.read((char *) & (last_position_), 8, 1);
		return;
	}

	void NamePositionPair::setName(ListObject &list_object) {
		for (int i = 0; i < length_; i++) {
			pair_[i].second = list_object.getName(i);
		}
		return;
	}

	void NamePositionPair::setName(std::string str, int index) {
		pair_[index].second = str;
		return;
	}

	void NamePositionPair::setToInvalid(int index) {
		pair_[index].first = -1;
		pair_[index].second = std::string(NAMELENGTH, '\xff');
		return;
	}

	int64_t NamePositionPair::getPosition(int index) {
		if (index < length_) {
			return pair_[index].first;
		} else {
			return last_position_;
		}
		
	}

	int64_t NamePositionPair::getLastPosition(){
		return last_position_;
	}

	std::string NamePositionPair::getName(int index) {
		return pair_[index].second;
	}

	void NamePositionPair::print(int index) {
		Rprintf("Index %d, Position %lf, String %s \n", index, (double)pair_[index].first, pair_[index].second.c_str());
	}

	// concatenate two pair vectors
	void NamePositionPair::merge(NamePositionPair & secondPair)
	{
		pair_.resize(length_ + secondPair.length_);
		for (int i = 0; i < secondPair.length_; i ++){
			pair_[i + length_]  = secondPair.pair_[i];
		}
		last_position_ = secondPair.last_position_;
		length_ = length_ + secondPair.length_;
	}

	// remove elements by given index object
	void NamePositionPair::remove(IndexObject & index_object) {
		std::vector<std::pair<int64_t, std::string> > pair_remain(length_ - index_object.getLength());
		int current_delete_index = 0;
		for (int i = 0; i < length_; i ++) {
			if (current_delete_index != index_object.getLength() && i == index_object.getIndex(current_delete_index)) {
				current_delete_index++;
				continue;
			} else {
				pair_remain[i - current_delete_index].first = getPosition(i);
				pair_remain[i - current_delete_index].second = getName(i);
			}
		}
		length_ = length_ - index_object.getLength();
		pair_ = pair_remain;
	}

	int NamePositionPair::getLength() {
		return length_;
	}

	bool NamePositionPair::cmp (std::pair<int64_t, std::string> const & a, std::pair<int64_t,std::string> const & b)
	{
		return a.second != b.second ?  a.second < b.second : a.first < b.first;
	};
}