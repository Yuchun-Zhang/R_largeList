
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
#include <math.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#undef ERROR
#include <R.h>
#include <Rinternals.h>

#define BYTE unsigned char
#define NAMELENGTH 16
#define CURRENT_VERSION ((0<<8) + (3<<4) + 0)
#define READABLE_VERSION ((0<<8) + (2<<4) + 1)
#define LIST_HEAD_POSITION 26
#define LENGTH_POSITION 30
#define MAXRETRIES 5
#define RETRYDELAY  250
#define HAS_NAME_POSITION 18


namespace large_list {
	class ConnectionFile;
	class UnitObject;
	class IndexObject;
	class ListObject;
	class NamePositionPair;
	class IndexWithValueObject;

	class Connection {
		virtual void create() = 0;
		virtual void connect() = 0;
		virtual void write(char *data, int nbytes, int nblocks) = 0;
		virtual void read(char *data, int nbytes, int nblocks) = 0;
	};

	class ConnectionFile {
	// This class handles all the file input/output staffs.
	private:
		FILE * fin_;
		FILE * fout_;
		char * file_dir_name_;
		// these two functions will be called in the connect function.
		void writeVersion();
		void checkVersion();
	public:
		// constructors/destructors
		ConnectionFile (SEXP file_name_sxp);
		~ConnectionFile();

		// build a connection to the file.
		void create();
		void connect();

		// wrapper for fwrite and fread, gets rid of warnings and handles exceptions.
		void write(char *data, int nbytes, int nblocks);
		void read(char *data, int nbytes, int nblocks);

		// set/read the pointers.
		void seekRead (int64_t position, int origin);
		void seekWrite (int64_t position, int origin);
		int64_t tellRead ();
		int64_t tellWrite ();

		// cut the file to shorter length.
		void cutFile();

		// move a data block.
		void moveData (const int64_t &move_from_start_pos, const int64_t &move_from_end_pos,
              const int64_t &move_to_start_pos, const int64_t &move_to_end_pos);
	};

	class UnitObject {
	// This class wraps a R object, handles the input/output/check etc. of the object.
	private:
		SEXP r_object_;
	public:
		// constructors/destructors
		UnitObject ();
		UnitObject (SEXP r_object);
		~UnitObject();	

		// operations to the R object.
		void set(SEXP r_object);
		void write(ConnectionFile & connection_file);
		void read(ConnectionFile & connection_file);
		SEXP get();
		int64_t calculateSerializedLength();
		void check();	

		// the following functions deal with the input/output/check/serialized length of R object.
		// they are static since they will be called recursively and it's not worthy to wrap up all r objects.
		static void getHeadInfo(SEXP _x, int &level, int &object, SEXP &attribute, SEXP &tag);
		static void checkSEXP(SEXP _x);
		static void writeLength(SEXP _x, ConnectionFile & connection_file);
		static void writeSEXP(SEXP _x, ConnectionFile & connection_file);
		static void readLength(ConnectionFile & connection_file, int &length);
		static SEXP readSEXP(ConnectionFile & connection_file);
		static void lengthOfSEXP(SEXP _x, int64_t & lenght);
		static std::string charsxpToString(SEXP char_sexp);
	};

	class MetaListObject {
	protected:
		bool has_name_;
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
		void writeNameBit (ConnectionFile & connection_file);
		void readNameBit (ConnectionFile & connection_file);
		void setNameBit (bool has_name);
		bool getNameBit ();

		// other
		void writeListHead (ConnectionFile & connection_file);

	};

	class ListObject : public MetaListObject{
	// This class contains mainly a vector of UnitObject and a string vector representing the names. It models the 
	// R list basically.
	private:
		std::vector<UnitObject> list_;
		std::vector<std::string> names_;
		std::vector<int64_t> serialized_length_;
	public:
		// constructors/destructors
		ListObject(int length);
		ListObject (SEXP list);
		ListObject();
		~ListObject();

		// size
		void resize(int length);

		// serialized length
		void calculateSerializedLength();
		int64_t getSerializedLength(int index);

		// name
		std::string getName(int index);
		void setName(std::string name, int index);
		static SEXP getObjectName(SEXP x, bool & has_name);

		// list of UnitObject
		void check();
		void write(ConnectionFile & connection_file, int index);
		void read(ConnectionFile & connection_file, int index);
		void set(SEXP r_object, int index);

		// other
		SEXP assembleRList (ConnectionFile & connection_file);
	};
   
	class NamePositionPair {
	// This is a position-name look up table.
	private:
		std::vector<std::pair<int64_t, std::string> > pair_;
		int length_;
		int64_t last_position_;
	public:
		// constructors/destructors
		NamePositionPair();
		NamePositionPair(int length);
		NamePositionPair(NamePositionPair &);
		~NamePositionPair();

		// file read/write 
		void read(ConnectionFile & connection_file);
		void read(ConnectionFile & connection_file, int index);
		void write(ConnectionFile & connection_file, bool write_last_position);

		// position
		void setPosition(ConnectionFile & connection_file, int i);
		void setPosition(int64_t position, int i);
		int64_t getPosition(int index);

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
		static bool cmp (std::pair<int64_t, std::string> const & a, std::pair<int64_t, std::string> const & b);

		// other
		void setToInvalid(int index);
		void merge(NamePositionPair &);
		void print(int index);
	};

	class IndexObject{
	// This class deals with the indicies when readling, removing, modifying the elements.
	protected:
		int list_length_;
		int length_;
		NamePositionPair pair_object_;
		std::vector<int> index_;
		std::vector<int> value_index_;
		// these two functions are called in the constructor
		void fileBinarySearchByName(ConnectionFile &connection_file, int64_t &position, std::string name, int &index, int length);
		void fileBinarySearchByPosition (ConnectionFile &connection_file, int64_t &position, int &index, int &length);
		void processNumeric();
	public:
		// constructors/destructors
		IndexObject(SEXP index, int list_length, ConnectionFile &connection_file, 
					bool extend_to_list_length = true);
		~IndexObject();

		// remove invalid value of the index.
		void removeInvalid ();

		// read in the position and names, i.e. the NamePositionPair object
		void readPair(ConnectionFile &connection_file);

		// get staffs 
		int64_t getPosition(int index);
		std::string getName(int index);
		int getIndex(int index);
		int getLength();

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
		IndexWithValueObject(SEXP index, int list_length, ConnectionFile &connection_file, 
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

	extern "C" SEXP saveList(SEXP object, SEXP file, SEXP append);
	extern "C" SEXP readList(SEXP file, SEXP index);
	extern "C" SEXP removeFromList(SEXP file, SEXP index);
	extern "C" SEXP modifyInList(SEXP file, SEXP index, SEXP object);
	extern "C" SEXP getListLength(SEXP file);
	extern "C" SEXP getListName(SEXP file);
	extern "C" SEXP modifyNameInList(SEXP file, SEXP index, SEXP names);
	extern "C" SEXP checkFileAndVersionExternal(SEXP file);
};
#endif //LARGE_LIST