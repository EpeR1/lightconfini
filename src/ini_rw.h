#ifndef INI_READ_H_INCLUDED
#define INI_READ_H_INCLUDED
// #include "deftypes.h"
#include <stdint.h> /* int64_t*/
 


typedef struct lci_data {

    enum nodeState {EMPTY, READY, CONTINUE, MULTILINE, ERROR } nodeState;
    int64_t  lineNum;
    int64_t  lineLen;

    char *section;
    int64_t  sectionLen; 
    int64_t  sectionStartPos;
    char *param;
    int64_t  paramLen;
    int64_t  paramStartPos;
    char *value;
    int64_t  valueLen;
    int64_t  valueStartPos;
    char *comment;
    int64_t  commentLen;
    int64_t  commentStartPos;
    char *errorMsg;
    int64_t  errorMsgLen;

    struct lci_data *next; 
} lci_data;

enum ini_states {Start, BgnSp, CommEndW, SectEndW, SectEndD, EqW1, EqW2, ValPSP, ValW, ValFSP, DqmW, Bslsh, Error, Stop };

struct lci_data *iniReadOut(const char *filename);
int64_t getFileMaxLineLen(FILE *tfd);
char *strResize(char *ptr, size_t oldsize, size_t newsize);
lci_data *destroyNodes( lci_data *head); 



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

