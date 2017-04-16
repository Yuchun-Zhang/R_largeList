
#ifndef LARGE_LIST
#define LARGE_LIST

#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#define PREDEF_PLATFORM_UNIX
#include <unistd.h>
#include <stdlib.h>
#endif

#if defined(_WIN32)
#define PREDEF_PLATFORM_WIN32
#include <windows.h>
#endif

#include <stdint.h>
#include <time.h>
#include <cstring>
#include <stdexcept>
#include <math.h>
#include <fstream>
#include <tuple>
#include <vector>
#include <algorithm>
#include <sstream>
#undef ERROR
#include <R.h>
#include <Rinternals.h>
#include <zlib.h>

#define BYTES_MASK (1<<1)
#define LATIN1_MASK (1<<2)
#define UTF8_MASK (1<<3)

#define NAMELENGTH 16
#define CURRENT_VERSION ((0<<8) + (3<<4) + 1)
#define READABLE_VERSION ((0<<8) + (2<<4) + 1)
#define LIST_HEAD_POSITION 26
#define LENGTH_POSITION 30
#define MAXRETRIES 5
#define RETRYDELAY  250
#define HAS_NAME_POSITION 18
#define IS_COMPRESS_POSITION 19
#define NUMBER_OF_MEM_SLOTS 100


namespace large_list {
	class ConnectionFile;
	class UnitObject;
	class IndexObject;
	class ListObject;
	class NamePositionTuple;
	class IndexWithValueObject;
	class MemorySlot;

	class Connection {
	public:
		virtual void write(const void *data, int nbytes, int nblocks) = 0;
		virtual void read(void *data, int nbytes, int nblocks) = 0;
		virtual void seekRead(int64_t position, int origin) = 0;
		virtual void seekWrite(int64_t position, int origin) = 0;
	};

	class ConnectionRaw : public Connection {
	private:
		void* raw_array_;
		int64_t read_pos_;
		int64_t write_pos_;
		int64_t length_;
	public:
		ConnectionRaw(MemorySlot &, int64_t length);
		~ConnectionRaw();
		void write(const void *data, int nbytes, int nblocks);
		void read(void *data, int nbytes, int nblocks);
		void seekRead(int64_t position, int origin);
		void seekWrite(int64_t position, int origin);
		void compress(MemorySlot &);
		void uncompress(MemorySlot &);
		void free(MemorySlot &);
		void* getRaw();
		int64_t getLength();
	};

	class ConnectionFile : public Connection {
		// This class handles all the file input/output staffs.
	private:
		std::FILE *fin_;
		std::FILE *fout_;
		char *file_dir_name_;
		// these two functions will be called in the connect function.
		void writeVersion();
		void checkVersion();
	public:
		// constructors/destructors
		ConnectionFile(SEXP file_name_sxp);
		~ConnectionFile();

		// build a connection to the file.
		void create();
		void connect();
		void disconnect();

		// wrapper for fwrite and fread, gets rid of warnings and handles exceptions.
		void write(const void *data, int nbytes, int nblocks);
		void read(void *data, int nbytes, int nblocks);

		// set/read the pointers.
		void seekRead(int64_t position, int origin);
		void seekWrite(int64_t position, int origin);
		int64_t tellRead();
		int64_t tellWrite();

		// cut the file to shorter length.
		void cutFile();

		// move a data block.
		void moveData(const int64_t &move_from_start_pos, 
					  const int64_t &move_from_end_pos,
					  const int64_t &move_to_start_pos, 
					  const int64_t &move_to_end_pos);
	};

	class UnitObject {
	// This class wraps a R object, handles the input/output/check etc. of the object.
	private:
		SEXP r_object_;
	public:
		// constructors/destructors
		UnitObject();
		UnitObject(SEXP r_object);
		~UnitObject();	

		// operations to the R object.
		void set(SEXP r_object);
		int64_t write(ConnectionFile & connection_file, MemorySlot &, bool is_compress);
		void read(ConnectionFile & connection_file, MemorySlot &, int64_t serialized_length, bool is_compress);
		SEXP get();
		int64_t calculateSerializedLength(MemorySlot &, bool is_compress);
		void check();	

		// the following functions deal with the input/output/check/serialized length of R object.
		// they are static since they will be called recursively and it's not worthy to wrap up all r objects.
		static void getHeadInfo(SEXP _x, int &level, int &object, SEXP &attribute, SEXP &tag);
		static void checkSEXP(SEXP _x);
		static void writeLength(SEXP _x, Connection & connection);
		static void writeSEXP(SEXP _x, Connection & connection);
		static void readLength(Connection & connection, int &length);
		static SEXP readSEXP(Connection & connection);
		static void lengthOfSEXP(SEXP _x, int64_t & lenght);
		static std::string charsxpToString(SEXP char_sexp);
	};

	class MetaListObject {
	protected:
		bool has_name_;
		bool is_compress_;
		int length_;
	public:
		MetaListObject();
		MetaListObject(int length);
		~MetaListObject();
		
		// length 
		void writeLength(ConnectionFile & connection_file);
		void readLength(ConnectionFile & connection_file);
		int getLength();
		void setLength(int length);

		// name bit
		void writeNameBit(ConnectionFile & connection_file);
		void readNameBit(ConnectionFile & connection_file);
		void setNameBit(bool has_name);
		bool getNameBit();

		// compress
		void writeCompressBit(ConnectionFile & connection_file);
		void readCompressBit(ConnectionFile & connection_file);
		void setCompressBit(bool is_compress);
		bool getCompressBit();

		// other
		void writeListHead(ConnectionFile & connection_file);

	};

	class ListObject : public MetaListObject{
	// This class contains mainly a vector of UnitObject and a string vector representing the names. It models the 
	// R list basically.
	private:
		PROTECT_INDEX ipx;
		SEXP r_list_;
		std::vector<std::string> names_;
		std::vector<int64_t> serialized_length_;
	public:
		// constructors/destructors
		ListObject(int length, bool is_compress = false);
		ListObject(SEXP list, bool is_compress = false);
		ListObject();
		~ListObject();

		// size
		void resize(int length);

		// serialized length
		void calculateSerializedLength(MemorySlot & memoryslot);
		void setSerializedLength(int64_t serialized_length, int index);
		int64_t getSerializedLength(int index);

		// name
		std::string getName(int index);
		void setName(std::string name, int index);
		static SEXP getObjectName(SEXP x, bool & has_name);


		// list of UnitObject
		void check();
		void write(ConnectionFile &, MemorySlot &, int index);
		void read(ConnectionFile &, MemorySlot &, int index);
		void set(SEXP r_object, int index);

		// other
		SEXP assembleRList ();
		void print();
	};
   
	class NamePositionTuple {
	// This is a position-name look up table.
	private:
		std::vector<std::tuple<int64_t, int64_t, std::string> > tuple_;
		int length_;
		int64_t last_position_;
	public:
		// constructors/destructors
		NamePositionTuple();
		NamePositionTuple(int length);
		NamePositionTuple(NamePositionTuple &);
		~NamePositionTuple();

		// file read/write 
		void read(ConnectionFile & connection_file);
		void read(ConnectionFile & connection_file, int index);
		void write(ConnectionFile & connection_file, bool write_last_position);

		// position
		void setPosition(ConnectionFile & connection_file, int index);
		void setPosition(int64_t position, int index);
		int64_t getPosition(int index);

		// length
		int64_t getSerializedLength(int index);
		void setSerializedLength(int64_t element_length, int index);

		// last position
		void setLastPosition(ConnectionFile & connection_file);
		void setLastPosition(int64_t last_position);
		void readLastPosition(ConnectionFile & connection_file);
		int64_t getLastPosition();

		// name
		void setName(ListObject &list_object);
		void setName(std::string, int index);
		std::string getName(int index);

		// length
		void resize(int length);		
		int getLength();

		// operations to the pair vector
		void sort();
		void remove(IndexObject & index_object);
		static bool cmp (std::tuple<int64_t, int64_t, std::string>  const & a, 
						 std::tuple<int64_t, int64_t, std::string>  const & b);

		// other
		void setToInvalid(int index);
		void merge(NamePositionTuple &);
		void print(int index);
	};

	class IndexObject{
	// This class deals with the indicies when readling, removing, modifying the elements.
	protected:
		int list_length_;
		int length_;
		NamePositionTuple tuple_object_;
		std::vector<int> index_;
		// these two functions are called in the constructor
		void fileBinarySearchByName(ConnectionFile &connection_file, 
									int64_t &position, 
									std::string name, 
									int &index, 
									int length);
		void fileBinarySearchByPosition(ConnectionFile &connection_file, 
										int64_t &position, 
										int &index, 
										int &length);
		void processNumeric();
	public:
		// constructors/destructors
		IndexObject(SEXP index, 
					int list_length, 
					ConnectionFile &connection_file, 
					bool extend_to_list_length = true);
		~IndexObject();

		// remove invalid value of the index.
		void removeInvalid();

		// read in the position and names, i.e. the NamePositionTuple object
		void readPair(ConnectionFile &connection_file);

		// get staffs 
		int64_t getPosition(int index);
		std::string getName(int index);
		int getIndex(int index);
		int getLength();
		int64_t getSerializedLength(int index);

		// others
		void sort();
		void removeDuplicate();
		void print(int type);
	};

	class IndexWithValueObject: public IndexObject {
	// This class inherits IndexObject, it can deal with indicies as well as the indicies of the value object.
	private:
		std::vector<std::pair<int, int> > index_pair_;
		int value_length_;
	public:
		// constructors/destructors
		IndexWithValueObject(SEXP index, 
							 int list_length, 
							 ConnectionFile &connection_file, 
							 bool extend_to_list_length = true);
		~IndexWithValueObject();

		// value length
		void setValueLength(int value_length);

		// value index
		void setValueIndex();
		int getValueIndex(int index);

		// overload the functions in IndexObject
		void sort();
		void removeInvalid ();
		void removeDuplicate();
		int getIndex(int index);
		void print();
		static bool cmp (std::pair<int, int> const & a, std::pair<int, int> const & b);
	};

	class ProgressReporter{
	private:
		clock_t clock_begin_;
		clock_t clock_end_;
		int estimated_sec_times_;
	public:
		bool is_long_time_;
		ProgressReporter();
		void reportProgress(int, int, std::string);
		void reportFinish(std::string);
		void clearLine();
	};

	class MemorySlot{
	private:
		bool is_slot_in_use[NUMBER_OF_MEM_SLOTS];
		bool is_slot_initialized[NUMBER_OF_MEM_SLOTS];
		int64_t slot_size[NUMBER_OF_MEM_SLOTS];
		void *slot[NUMBER_OF_MEM_SLOTS];
	public:
		MemorySlot();
		~MemorySlot();
		void* slot_malloc(int64_t);
		void slot_free(void *);
		void* slot_realloc(void*, int64_t);
	};

	extern "C" SEXP saveList(SEXP object, SEXP file, SEXP append, SEXP compress, SEXP verbose);
	extern "C" SEXP readList(SEXP file, SEXP index, SEXP verbose);
	extern "C" SEXP removeFromList(SEXP file, SEXP index, SEXP verbose);
	extern "C" SEXP modifyInList(SEXP file, SEXP index, SEXP object, SEXP verbose);
	extern "C" SEXP getListLength(SEXP file);
	extern "C" SEXP getListName(SEXP file);
	extern "C" SEXP modifyNameInList(SEXP file, SEXP index, SEXP names);
	extern "C" SEXP isListCompressed(SEXP file);
	extern "C" SEXP checkFileAndVersionExternal(SEXP file);
	extern "C" SEXP checkList(SEXP object);
	extern "C" SEXP largeListTest();
}
#endif //LARGE_LIST