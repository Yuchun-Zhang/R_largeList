#include <stdlib.h>
#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>
#include <R_ext/Visibility.h>  

#define CALLDEF(name, n)  {#name, (DL_FUNC) &name, n}

extern SEXP saveList(SEXP object, SEXP file, SEXP append, SEXP compress, SEXP verbose);
extern SEXP readList(SEXP file, SEXP index, SEXP verbose);
extern SEXP removeFromList(SEXP file, SEXP index, SEXP verbose);
extern SEXP modifyInList(SEXP file, SEXP index, SEXP object, SEXP verbose);
extern SEXP getListLength(SEXP file);
extern SEXP getListName(SEXP file);
extern SEXP modifyNameInList(SEXP file, SEXP index, SEXP names);
extern SEXP isListCompressed(SEXP file);
extern SEXP checkFileAndVersionExternal(SEXP file);
extern SEXP checkList(SEXP object);
extern SEXP largeListTest();

const static R_CallMethodDef R_CallDef[] = {
    CALLDEF(saveList, 5),
    CALLDEF(readList, 3),
    CALLDEF(removeFromList, 3),
    CALLDEF(modifyInList, 4),
    CALLDEF(getListLength, 1),
    CALLDEF(getListName, 1),
    CALLDEF(modifyNameInList, 3),
    CALLDEF(isListCompressed, 1),
    CALLDEF(checkFileAndVersionExternal, 1),
    CALLDEF(checkList, 1),
    CALLDEF(largeListTest, 0),
    {NULL, NULL, 0}
};

void attribute_visible R_init_largeList(DllInfo *dll) {
    R_registerRoutines(dll, NULL, R_CallDef, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
    R_forceSymbols(dll, TRUE);
}