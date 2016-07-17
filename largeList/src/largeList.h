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
#undef ERROR
#include <R.h>
#include <Rinternals.h>
#include <Rversion.h>

#define BYTE unsigned char
#define NAMELENGTH 16


//writeObject.cpp
inline void  getHeadInfo(SEXP, int &, SEXP &, SEXP &);
inline void  writeREALSXP(SEXP, FILE *);
inline void  writeNILSXP(FILE *);
inline void  writeINTSXP(SEXP, FILE *);
inline void  writeCPLXSXP(SEXP, FILE *);
inline void  writeLGLSXP(SEXP, FILE *);
inline void  writeRAWSXP(SEXP, FILE *);
inline void  writeSTRSXP(SEXP, FILE *);
inline void  writeCHARSXP(SEXP, FILE *);
inline void  writeSYMSXP(SEXP, FILE *);
inline void  writeVECSXP(SEXP, FILE *);
inline void  writeATTR(SEXP, FILE *);
inline void  writeHead(SEXP, int, int, SEXP, SEXP, FILE *);
inline void  writeLength(SEXP, FILE *);
int writeSEXP(SEXP, FILE *);

//readObject.cpp
SEXP readSEXP(FILE *);
inline SEXP readREALSXP(FILE *);
inline SEXP readINTSXP(FILE *);
inline SEXP readRAWSXP(FILE *);
inline SEXP readLGLSXP(FILE *);
inline SEXP readCHARSXP(FILE *);
inline SEXP readSTRSXP(FILE *);
inline SEXP readVECSXP(FILE *);
inline SEXP readSYMSXP(FILE *);
inline void readHead(FILE *, int &, int &, int &, int &, int &);
inline void readLength(FILE *, int &);
inline void readATTR(SEXP &, FILE *);

//utils.cpp
bool cmp (std::pair<std::string, int64_t> const & , std::pair<std::string, int64_t> const & );
void fileBinarySearch (FILE *, int64_t &, std::string &, int &, int &);
void fileBinarySearchIndex (FILE *, int64_t &, int &, int &);
void getPositionByIndex(FILE *, int &, int &, int64_t &);
void writeVersion (FILE *);
SEXP getObjectName(SEXP);
void writeItemIdx(std::vector<std::pair<std::string, int64_t> > &, FILE* , int &);
void readItemIdx(std::vector<std::pair<std::string, int64_t> > &, FILE* , int &);
void mergeTwoSortedItemIdx(std::vector<std::pair<std::string, int64_t> > &,
                           std::vector<std::pair<std::string, int64_t> > &,
                           std::vector<std::pair<std::string, int64_t> > &);
bool checkFile(const char *);
const char* getFullPath(SEXP);
void cutFile(const char *, const int64_t &);

//export files.
extern "C" SEXP saveList(SEXP, SEXP, SEXP);
extern "C" SEXP removeFromList(SEXP, SEXP);
extern "C" SEXP readList(SEXP, SEXP);
extern "C" SEXP getListName(SEXP);
extern "C" SEXP getListLength(SEXP);