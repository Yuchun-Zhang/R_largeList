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
#undef ERROR
#include <Rcpp.h> 

#define BYTE unsigned char
#define NAMELENGTH 8

// RcppExport SEXP int2RawVec(int64_t, int  bitLength = 4);
// int64_t rawVec2Int(SEXP, int  bitLength = 4);
int getObjectName(SEXP, Rcpp::StringVector &);
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

int readSEXP(SEXP &, std::fstream &);
int readREALSXP(SEXP &, std::fstream &, int &);
int readINTSXP(SEXP &, std::fstream &, int &);
int readRAWSXP(SEXP &, std::fstream &, int &);
int readLGLSXP(SEXP &, std::fstream &, int &);
int readCHARSXP(SEXP &, std::fstream &);
int readSTRSXP(SEXP &, std::fstream &, int &);
int readVECSXP(SEXP &, std::fstream &, int &);
int readSYMSXP(SEXP &, std::fstream &, int &);
int readHead(std::fstream &, int &, int &, int &, int &, int &);
int readLength(std::fstream &, int &);
int readATTR(SEXP &, std::fstream &);
bool cmp (std::pair<std::string, int64_t> const & , std::pair<std::string, int64_t> const & );
void fileBinarySearch (std::fstream &, int64_t &, std::string &, int &, int &);
void fileBinarySearchIndex (std::fstream &, int64_t &, int &, int &);
void getPositionByIndex(std::fstream &, int &, int &, int64_t &);