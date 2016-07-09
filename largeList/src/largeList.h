#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#define PREDEF_PLATFORM_UNIX
#include <unistd.h>
#endif

#if defined(_WIN32)
#define PREDEF_PLATFORM_WIN32
#include <windows.h>
#endif

#include <math.h>
#include <iostream> 
#include <fstream>
#include <vector>
#undef ERROR
#include <R.h>
#include <Rinternals.h>
#include <Rversion.h>

#define BYTE unsigned char
#define NAMELENGTH 8

int writeNameAttrHead( std::fstream &);
int writeNameAttrEnd( std::fstream &);
int writeNameAttrLength(int &, std::fstream &);
int getHeadInfo(SEXP, int &, SEXP &, SEXP &);
int writeREALSXP(SEXP, std::fstream &);
int writeNILSXP(std::fstream &);
int writeINTSXP(SEXP, std::fstream &);
int writeLGLSXP(SEXP, std::fstream &);
int writeRAWSXP(SEXP, std::fstream &);
int writeSTRSXP(SEXP, std::fstream &);
int writeCHARSXP(SEXP, std::fstream &);
int writeSEXP(SEXP, std::fstream &);
int writeSYMSXP(SEXP, std::fstream &);
int writeVECSXP(SEXP, std::fstream &);
int writeATTR(SEXP, std::fstream &);
int writeSYMSXP(SEXP, std::fstream &);
int writeHead(SEXP, int, int, SEXP, SEXP, std::fstream &);
int writeLength(SEXP, std::fstream &);
int writeSEXP(SEXP, std::fstream &);
int writeHeadAndLength(SEXP, std::fstream &);

SEXP readSEXP(std::fstream &);
SEXP readREALSXP(std::fstream &);
SEXP readINTSXP(std::fstream &);
SEXP readRAWSXP(std::fstream &);
SEXP readLGLSXP(std::fstream &);
SEXP readCHARSXP(std::fstream &);
SEXP readSTRSXP(std::fstream &);
SEXP readVECSXP(std::fstream &);
SEXP readSYMSXP(std::fstream &);
void readHead(std::fstream &, int &, int &, int &, int &, int &);
void readLength(std::fstream &, int &);
void readATTR(SEXP &, std::fstream &);

bool cmp (std::pair<std::string, int64_t> const & , std::pair<std::string, int64_t> const & );
void fileBinarySearch (std::fstream &, int64_t &, std::string &, int &, int &);
void fileBinarySearchIndex (std::fstream &, int64_t &, int &, int &);
void getPositionByIndex(std::fstream &, int &, int &, int64_t &);
void writeVersion (std::fstream &);
SEXP getObjectName(SEXP);

extern "C" SEXP saveList(SEXP, SEXP, SEXP);
extern "C" SEXP removeFromList(SEXP, SEXP);
extern "C" SEXP readList(SEXP, SEXP);
extern "C" SEXP getListName(SEXP);
extern "C" SEXP getListLength(SEXP);