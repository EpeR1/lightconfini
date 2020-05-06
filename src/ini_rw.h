#ifndef INI_READ_H_INCLUDED
#define INI_READ_H_INCLUDED
// #include "deftypes.h"
#include <stdint.h> /* int64_t*/



typedef struct lci_data {

    enum nodeState {EMPTY, READY, CONTINUE, MULTILINE, ERROR } nodeState;
    uint64_t  lineNum;
    uint64_t  lineLen;

    char *section;
    uint64_t  sectionLen;
    uint64_t  sectionStartPos;
    char *param;
    uint64_t  paramLen;
    uint64_t  paramStartPos;
    char *value;
    uint64_t  valueLen;
    uint64_t  valueStartPos;
    char *comment;
    uint64_t  commentLen;
    uint64_t  commentStartPos;
    char *errorMsg;
    uint64_t  errorMsgLen;

    struct lci_data *next; 
} lci_data;

enum ini_states {Start, BgnSp, CommEndW, SectEndW, SectEndD, EqW1, EqW2, ValPSP, ValW, ValFSP, DqmW, Bslsh, Error, Stop };

struct lci_data *iniReadOut(const char *filename);
int64_t getFileMaxLineLen(FILE *tfd);



char     *lciGETtoStr( const char *section, const char *param, char *dest, size_t dstlen );
//int      lciGETtoStrlen(const char *section, const char *param, ...);
int8_t     lciGETtoInt8( const char *filename, const char *section, const char *param);
int16_t    lciGETtoInt16(const char *filename, const char *section, const char *param);
int32_t    lciGETtoInt32(const char *filename, const char *section, const char *param);
int64_t    lciGETtoInt64(const char *filename, const char *section, const char *param);

double   lciGETtoDlb(const char *filename, const char *section, const char *param);
float    lciGETtoFlt(const char *filename, const char *section, const char *param);
long int lciGETtoLng(const char *filename, const char *section, const char *param);




#endif // INI_READ_H_INCLUDED

