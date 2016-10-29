#include "large_list.h"
namespace large_list {

	ConnectionRaw::ConnectionRaw(int64_t length) {
		 raw_array_ = (char*) malloc (length);
		 length_ = length;
		 read_pos_ = 0;
		 write_pos_ = 0;
	};
	
	ConnectionRaw::~ConnectionRaw() {
		 free(raw_array_);
	};

	void ConnectionRaw::seekRead(int64_t position, int origin) {
		int64_t origin_position;
		switch(origin) {
			case SEEK_SET : {origin_position = 0; break;}
			case SEEK_END : {origin_position = length_; break;}
			case SEEK_CUR : {origin_position = read_pos_; break;}
		}
		read_pos_ = origin_position + position;
		return;
	}

	void ConnectionRaw::seekWrite(int64_t position, int origin) {
		int64_t origin_position;
		switch(origin) {
			case SEEK_SET : {origin_position = 0; break;}
			case SEEK_END : {origin_position = length_; break;}
			case SEEK_CUR : {origin_position = write_pos_; break;}
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

	void ConnectionRaw::compress() {
		char * raw_array_compressed;
		int64_t compress_bound = compressBound(length_);
		raw_array_compressed = (char*) malloc (compress_bound);
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
	    if(res != Z_STREAM_END) {throw std::runtime_error("internal error in compress");}
	    int64_t length_after_compress = compress_bound - strm.avail_out;
	    // Rprintf("After Compress %ld \n", length_after_compress);
	    deflateEnd(&strm);

	    free(raw_array_);
	    raw_array_  = raw_array_compressed;
	    length_ = length_after_compress;
		return;
	}

	void ConnectionRaw::uncompress() {
		char * raw_array_uncompressed;
		int64_t uncompress_bound = 3 * length_;
		raw_array_uncompressed = (char*) malloc (uncompress_bound);
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
	    		raw_array_uncompressed = (char *) realloc(raw_array_uncompressed, uncompress_bound);
	    		continue; 
	    	}
	    	if (res == Z_STREAM_END) {break;}
	    	num_try++;
	    	// Rprintf("Zlib Error %d \n", res);
	    }
	    int64_t length_after_uncompress = uncompress_bound - strm.avail_out;
	    // Rprintf("After Uncompress %ld \n", length_after_uncompress);
	    inflateEnd(&strm);

	    free(raw_array_);
	    raw_array_  = raw_array_uncompressed;
	    length_ = length_after_uncompress;
		return;
	}

	char* ConnectionRaw::getRaw() {
		return raw_array_;
	}
	int64_t ConnectionRaw::getLength() {
		return length_;
	};
}