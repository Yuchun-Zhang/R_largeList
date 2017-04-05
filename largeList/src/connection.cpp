#include "large_list.h"
namespace large_list {
	
	ConnectionFile::ConnectionFile (SEXP file_name_sxp) {
		const char *file_name_relative = CHAR(STRING_ELT(file_name_sxp, 0));
		file_dir_name_ = (char *)R_ExpandFileName(file_name_relative);
		fin_ = NULL;
		fout_ = NULL;
		//Rprintf("%s ", file_dir_name_);
	}

	ConnectionFile::~ConnectionFile() {
		// Rprintf("Begin to destruct file connection! \n");
		if (fin_) { std::fclose(fin_);}
		if (fout_) { std::fclose(fout_);}
		// Rprintf("FielIO object successfully destructed! \n");
	}

	// build a connection to the file, set fin_ and fout_.
	void ConnectionFile::create() {
		fout_ = std::fopen(file_dir_name_, "wb");
		if (fout_ == NULL) {
			throw std::runtime_error("directory does not exist.");
		}
		fin_ = std::fopen(file_dir_name_, "rb");
		writeVersion();
		return;
	}

	// connect to exsiting file
	void ConnectionFile::connect() {
		fout_ = std::fopen(file_dir_name_, "r+b");
		if (fout_ == NULL) {
			throw std::runtime_error("file does not exist.");
		}
		fin_ = std::fopen(file_dir_name_, "rb");
		checkVersion();
		return;
	}

	// fclose the pointers and reset them to NULL
	void ConnectionFile::disconnect() {
		if (fin_) { std::fclose(fin_); fin_ = NULL;}
		if (fout_) { std::fclose(fout_); fout_ = NULL;}
		return;
	}
	
	//safe write
	void ConnectionFile::write(char *data, int nbytes, int nblocks) {
		int64_t initial_ptr_position = std::ftell(fout_);
		int retries = 0;
		while (((int)std::fwrite(data, nbytes, nblocks, fout_) != nblocks) && (retries < MAXRETRIES)) {
			std::fseek(fout_, initial_ptr_position, SEEK_SET);
			retries ++;
		}
		if (retries == MAXRETRIES) {
			throw std::runtime_error("fwrite failed!");
		}
		return;
	}


	//safe fread
	void ConnectionFile::read(char *data, int nbytes, int nblocks) {
		int64_t initial_ptr_position = std::ftell(fin_);
		int retries = 0;
		while (((int)std::fread(data, nbytes, nblocks, fin_) != nblocks) && (retries < MAXRETRIES)) {
			std::fseek(fin_, initial_ptr_position, SEEK_SET);
			retries ++;
		}
		if (retries == MAXRETRIES) {
			throw std::runtime_error("fread failed!");
		}
		return;
	}
 
	void ConnectionFile::seekRead (int64_t position, int origin) {
		std::fseek(fin_, position, origin);
		return;
	}

	void ConnectionFile::seekWrite (int64_t position, int origin) {
		std::fseek(fout_, position, origin);
		return;
	}

	int64_t ConnectionFile::tellRead() {
		return(std::ftell(fin_));
	}

	int64_t ConnectionFile::tellWrite() {
		return(std::ftell(fout_));
	}	

	//writeVersion
	void ConnectionFile::writeVersion () {
		std::string head("LARGELIST ");
		write((char*)head.c_str(), 1, 10);
		int current_version = CURRENT_VERSION;
		int readable_version = READABLE_VERSION;
		write((char *)&current_version, 1, 4);
		write((char *)&readable_version, 1, 4);
		int has_name = 0;
		write((char *)&has_name, 1, 1);
		std::string reserved_string(7, '\x00');
		write((char*)reserved_string.c_str(), 1, 7);
		return;
	}

	//CheckVersion
	void ConnectionFile::checkVersion() {
		std::fseek(fin_, 0, SEEK_END);
		int64_t length_of_file = std::ftell(fin_);
		if (length_of_file < 26) { throw std::runtime_error("unkown file format!"); }
		std::string right_head("LARGELIST ");
		std::string head(10, '\x00');
		std::fseek(fin_, 0, SEEK_SET);
		read((char *)&head[0], 1, 10);
		if (right_head.compare(head) != 0) { throw std::runtime_error("unkown file format!"); }
		std::fseek(fin_, 10, SEEK_SET);
		int current_version;
		read((char *)&current_version, 4, 1);
		if (current_version < READABLE_VERSION) {
			std::ostringstream msg;
			msg << "can only read files created by version above " 
				<< ((READABLE_VERSION >> 8) & 0x0f) << "."
				<< ((READABLE_VERSION >> 4) & 0x0f) << "."
				<< ((READABLE_VERSION >> 0) & 0x0f);
			throw std::runtime_error(msg.str());
		}
		return;
	}

	//cutFile
	void ConnectionFile::cutFile() {
		int64_t file_length = std::ftell(fout_);
	  disconnect();
#if defined PREDEF_PLATFORM_UNIX
		// Rprintf("Begin to truncate \n");
		if (truncate(file_dir_name_, file_length) != 0) {
			throw std::runtime_error("file truncation failed (Unix).");
		}
		// Rprintf("Truncate finished \n");
#endif

#if defined PREDEF_PLATFORM_WIN32
		LARGE_INTEGER file_length_w;
		file_length_w.QuadPart = file_length;
		int retries = 0;
		HANDLE fh;
		do {
			fh = CreateFile((LPCTSTR)file_dir_name_,
			                GENERIC_WRITE, // open for write
			                0,
			                NULL, // default security
			                OPEN_EXISTING, // existing file only
			                FILE_ATTRIBUTE_NORMAL, // normal file
			                NULL);
			if (fh == INVALID_HANDLE_VALUE) {
				DWORD last_error = GetLastError();
				if (ERROR_SHARING_VIOLATION == last_error) {
					retries += 1;
					Sleep(RETRYDELAY);
					continue;
				} else {
				  char error_info[200];
				  sprintf(error_info, 
                  "file truncation failed (Windows), get file handle error. Error Code %ld .", 
                  last_error);
					throw std::runtime_error(error_info);
				}
			}
			break;
		} while (retries < MAXRETRIES);
		if (fh == INVALID_HANDLE_VALUE) {
			throw std::runtime_error("tried to trauncate file but it was already in use");
		}

		SetFilePointerEx(fh, file_length_w, NULL, 0);
		if (SetEndOfFile(fh) == 0) {
			DWORD last_error = GetLastError();
		  char error_info[200];
		  sprintf(error_info, 
            "file truncation failed (Windows), Error Code %ld .", 
            last_error);
		  throw std::runtime_error(error_info);
		}
		CloseHandle(fh);
#endif
		return;
	}

	
	void ConnectionFile::moveData(const int64_t &move_from_start_pos, const int64_t &move_from_end_pos,
	const int64_t &move_to_start_pos, const int64_t &move_to_end_pos) {
		if (move_from_end_pos - move_from_start_pos != move_to_end_pos - move_to_start_pos) {return;}
		BYTE *to_move_raw = (BYTE *) std::malloc((move_from_end_pos - move_from_start_pos) * sizeof(BYTE));
		// std::vector<BYTE> to_move_raw(move_from_end_pos - move_from_start_pos);
		seekRead(move_from_start_pos, SEEK_SET);
		// Rprintf("to read");
		read((char*) & (to_move_raw[0]), 1, move_from_end_pos - move_from_start_pos);
		seekWrite(move_to_start_pos, SEEK_SET);
		// Rprintf("to write");
		write((char*) & (to_move_raw[0]), 1, move_to_end_pos - move_to_start_pos);
		std::free(to_move_raw);
		return;
	}


//-----------------------------------------  Connection Raw ---------------------------------//

	ConnectionRaw::ConnectionRaw(MemorySlot & memory_slot, int64_t length) {
		raw_array_ = memory_slot.slot_malloc(length);
		length_ = length;
		read_pos_ = 0;
		write_pos_ = 0;
	}
	
	ConnectionRaw::~ConnectionRaw() {
	}

	void ConnectionRaw::free(MemorySlot & memory_slot) {
		memory_slot.slot_free(raw_array_);
	}

	void ConnectionRaw::seekRead(int64_t position, int origin) {
		int64_t origin_position;
	  origin_position = 0;
		switch (origin) {
			case SEEK_SET : {origin_position = 0; break;}
			case SEEK_END : {origin_position = length_; break;}
			case SEEK_CUR : {origin_position = read_pos_; break;}
		  default : {origin_position = 0; break;}
		}
		read_pos_ = origin_position + position;
		return;
	}

	void ConnectionRaw::seekWrite(int64_t position, int origin) {
		int64_t origin_position;
	  origin_position = 0;
		switch (origin) {
			case SEEK_SET : {origin_position = 0; break;}
			case SEEK_END : {origin_position = length_; break;}
			case SEEK_CUR : {origin_position = write_pos_; break;}
		  default : {origin_position = 0; break;}
		}
		write_pos_ = origin_position + position;
		return;
	}

	void ConnectionRaw::read(char *data, int nbytes, int nblocks) {
		std::memcpy(data, &raw_array_[read_pos_], nbytes * nblocks);
		read_pos_ += nbytes * nblocks;
		return;
	}

	void ConnectionRaw::write(char *data, int nbytes, int nblocks) {
		std::memcpy(&raw_array_[write_pos_], data, nbytes * nblocks);
		write_pos_ += nbytes * nblocks;
		return;
	}

	void ConnectionRaw::compress(MemorySlot & memory_slot) {
		char * raw_array_compressed;
		int64_t compress_bound = compressBound(length_);
		raw_array_compressed = memory_slot.slot_malloc(compress_bound);
		int res;

		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;

		strm.avail_in = length_; // size of input
		strm.next_in = (Bytef *) raw_array_; // input array
		strm.avail_out = compress_bound; // size of output
		strm.next_out = (Bytef *) raw_array_compressed; // output array

		// Rprintf("Before Compress %ld \n", length_);
		deflateInit(&strm, Z_DEFAULT_COMPRESSION);
		res = deflate(&strm, Z_FINISH);
		if (res != Z_STREAM_END) {throw std::runtime_error("internal error in compress");}
		int64_t length_after_compress = compress_bound - strm.avail_out;
		// Rprintf("After Compress %ld \n", length_after_compress);
		deflateEnd(&strm);

		memory_slot.slot_free(raw_array_);
		raw_array_  = raw_array_compressed;
		length_ = length_after_compress;
		return;
	}

	void ConnectionRaw::uncompress(MemorySlot & memory_slot) {
		char * raw_array_uncompressed;
		int64_t uncompress_bound = 3 * length_;
		raw_array_uncompressed = memory_slot.slot_malloc(uncompress_bound);
		int res;

		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;

		// Rprintf("Before Uncompress %ld \n", length_);
		inflateInit(&strm);
		int num_try = 1;
		while (num_try <= 5) {
			inflateReset(&strm);
			strm.avail_in = length_; // size of input
			strm.next_in = (Bytef *) raw_array_; // input array
			strm.avail_out = uncompress_bound; // size of output
			strm.next_out = (Bytef *) raw_array_uncompressed; // output array
			res = inflate(&strm, Z_FINISH);
			if (res == Z_BUF_ERROR) {
				// Rprintf("uncompress_bound %ld \n", uncompress_bound);
				uncompress_bound *= 2;
				raw_array_uncompressed = memory_slot.slot_realloc(raw_array_uncompressed, uncompress_bound);
				continue;
			}
			if (res == Z_STREAM_END) {break;}
			num_try++;
			// Rprintf("Zlib Error %d \n", res);
		}
		int64_t length_after_uncompress = uncompress_bound - strm.avail_out;
		// Rprintf("After Uncompress %ld \n", length_after_uncompress);
		inflateEnd(&strm);

		memory_slot.slot_free(raw_array_);
		raw_array_  = raw_array_uncompressed;
		length_ = length_after_uncompress;
		return;
	}

	char* ConnectionRaw::getRaw() {
		return raw_array_;
	}

	int64_t ConnectionRaw::getLength() {
		return length_;
	}
}