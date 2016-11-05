#include "large_list.h"
namespace large_list {

	NamePositionTuple::NamePositionTuple() {}

	NamePositionTuple::NamePositionTuple(int length) {
		length_ = length;
		tuple_.resize(length_, std::tuple<int64_t, int64_t, std::string> (0, 0, ""));
		last_position_ = 0;
	}

	// initialize by deep copy
	NamePositionTuple::NamePositionTuple(NamePositionTuple & toCopyObject) {
		length_ = toCopyObject.getLength();
		tuple_.resize(length_, std::tuple<int64_t, int64_t, std::string> (0, 0, ""));
		for (int i = 0; i < length_; i++) {
			std::get<0>(tuple_[i]) = toCopyObject.getPosition(i);
			std::get<1>(tuple_[i]) = toCopyObject.getSerializedLength(i);
			std::get<2>(tuple_[i]) = toCopyObject.getName(i);
		}
		last_position_ = toCopyObject.getLastPosition();
	}

	NamePositionTuple::~NamePositionTuple() {}

	void NamePositionTuple::resize(int length) {
		length_ = length;
		tuple_.resize(length_, std::tuple<int64_t, int64_t, std::string> (0, 0, ""));
	}

	// write the whole table
	void NamePositionTuple::write(ConnectionFile & connection_file, bool write_last_position) {
		for (int i = 0; i < length_; i++) {
			connection_file.write((char *) & (std::get<0>(tuple_[i])), 8, 1);
			connection_file.write((char*) std::get<2>(tuple_[i]).c_str(), NAMELENGTH, 1);
		}
		if (write_last_position) {
			connection_file.write((char *) & (last_position_), 8, 1);
		}
		return;
	}

	// read the whole table 
	void NamePositionTuple::read(ConnectionFile & connection_file) {
		connection_file.seekRead(-(8 + NAMELENGTH) * length_ * 2 - 8, SEEK_END);
		for (int i = 0; i < length_; i++) {
			connection_file.read((char *) & (std::get<0>(tuple_[i])), 8, 1);
			std::get<2>(tuple_[i]).resize(NAMELENGTH);
			connection_file.read((char *) & (std::get<2>(tuple_[i])[0]), NAMELENGTH, 1);
		}
		readLastPosition(connection_file);
		for (int i = 0; i < length_ - 1; i++) {
			std::get<1>(tuple_[i]) = std::get<0>(tuple_[i + 1]) - std::get<0>(tuple_[i]);
		}
		if (length_ > 0) {
			std::get<1>(tuple_[length_ - 1]) = last_position_ - std::get<0>(tuple_[length_ - 1]);
		}
		return;
	}

	// read one element
	void NamePositionTuple::read(ConnectionFile & connection_file, int index) {
		connection_file.read((char *) & std::get<0>(tuple_[index]), 8, 1);
		std::get<2>(tuple_[index]).resize(NAMELENGTH);
		connection_file.read((char *) & (std::get<2>(tuple_[index])[0]), NAMELENGTH, 1);
		int64_t next_position;
		connection_file.read((char *) & next_position, 8, 1);
		std::get<1>(tuple_[index]) = next_position - std::get<0>(tuple_[index]);
		return;
	}

	// sort first by name than by position
	void NamePositionTuple::sort() {
		std::stable_sort(tuple_.begin(), tuple_.end(), cmp);
		return;
	}

	void NamePositionTuple::setPosition(ConnectionFile & connection_file, int index) {
		std::get<0>(tuple_[index]) = connection_file.tellWrite();
		return;
	}

	void NamePositionTuple::setPosition(int64_t position, int index) {
		std::get<0>(tuple_[index]) = position;
		return;
	}

	void NamePositionTuple::setLastPosition(ConnectionFile & connection_file) {
		last_position_ = connection_file.tellWrite();
		return;
	}

	void NamePositionTuple::setLastPosition(int64_t last_position) {
		last_position_ = last_position;
		return;
	}

	void NamePositionTuple::readLastPosition(ConnectionFile & connection_file) {
		connection_file.seekRead(-(8 + NAMELENGTH) * length_ - 8, SEEK_END);
		connection_file.read((char *) & (last_position_), 8, 1);
		return;
	}

	void NamePositionTuple::setName(ListObject & list_object) {
		for (int i = 0; i < length_; i++) {
			std::get<2>(tuple_[i]) = list_object.getName(i);
		}
		return;
	}

	void NamePositionTuple::setName(std::string str, int index) {
		std::get<2>(tuple_[index]) = str;
		return;
	}

	int64_t NamePositionTuple::getSerializedLength(int index) {
		return std::get<1>(tuple_[index]);
	}

	void NamePositionTuple::setSerializedLength(int64_t element_length, int index) {
		std::get<1>(tuple_[index]) = element_length;
		return;
	}

	void NamePositionTuple::setToInvalid(int index) {
		std::get<0>(tuple_[index]) = -1;
		std::get<1>(tuple_[index]) = -1;
		std::get<2>(tuple_[index]).assign(NAMELENGTH, '\xff');
		return;
	}

	int64_t NamePositionTuple::getPosition(int index) {
		if (index < length_) {
			return std::get<0>(tuple_[index]);
		} else {
			return last_position_;
		}

	}

	int64_t NamePositionTuple::getLastPosition() {
		return last_position_;
	}

	std::string NamePositionTuple::getName(int index) {
		return std::get<2>(tuple_[index]);
	}

	void NamePositionTuple::print(int index) {
		Rprintf("Index %d, Position %lf, Serialized_lentth %lf, String %s \n",
		        index,
		        (double)std::get<0>(tuple_[index]),
		        (double)std::get<1>(tuple_[index]),
		        std::get<2>(tuple_[index]).c_str());
	}

	// concatenate two pair vectors
	void NamePositionTuple::merge(NamePositionTuple & secondTuple)
	{
		tuple_.resize(length_ + secondTuple.length_);
		for (int i = 0; i < secondTuple.length_; i ++) {
			tuple_[i + length_]  = secondTuple.tuple_[i];
		}
		last_position_ = secondTuple.last_position_;
		length_ = length_ + secondTuple.length_;
	}

	// remove elements by given index object
	void NamePositionTuple::remove(IndexObject & index_object) {
		std::vector<std::tuple<int64_t, int64_t, std::string> > tuple_remain(length_ - index_object.getLength());
		int current_delete_index = 0;
		for (int i = 0; i < length_; i ++) {
			if (current_delete_index != index_object.getLength() && i == index_object.getIndex(current_delete_index)) {
				current_delete_index++;
				continue;
			} else {
				std::get<0>(tuple_remain[i - current_delete_index]) = getPosition(i);
				std::get<1>(tuple_remain[i - current_delete_index]) = getSerializedLength(i);
				std::get<2>(tuple_remain[i - current_delete_index]) = getName(i);
			}
		}
		length_ = length_ - index_object.getLength();
		tuple_ = tuple_remain;
	}

	int NamePositionTuple::getLength() {
		return length_;
	}

	bool NamePositionTuple::cmp (std::tuple<int64_t, int64_t, std::string>  const & a,
	                             std::tuple<int64_t, int64_t, std::string>  const & b) {
		return std::get<2>(a) != std::get<2>(b) ?  std::get<2>(a) < std::get<2>(b) : std::get<0>(a) < std::get<0>(b);
	}
}