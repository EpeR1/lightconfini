#ifndef INI_READ_H_INCLUDED
#define INI_READ_H_INCLUDED
/* #include "deftypes.h" */
#include <stdint.h> /* int64_t*/
 
 

typedef struct lci_data {

    enum nodeState {lci_EMPTY, lci_READY, lci_CONTINUE, lci_MULTILINE, lci_ERROR } nodeState;
    int32_t  lineNum;
    int32_t  lineLen;

    char *section;
    int32_t  sectionLen; 
    int32_t  sectionStartPos;
    char *param;
    int32_t  paramLen;
    int32_t  paramStartPos;
    char *value;
    int32_t  valueLen;
    int32_t  valueStartPos;
    enum valueDraw {lci_EMPTYVAL, lci_SIMPLEVAL, lci_MULTILINEVAL, lci_DQUOTEDVAL} valueDraw;
    char *comment;
    int32_t  commentLen;
    int32_t  commentStartPos;
    char     commentSign;
    char *errorMsg;
    int32_t  errorMsgLen;

    struct lci_data *next; 
} lci_data;



struct lci_data *iniReadOut(const char *filename);
/* int64_t getFileMaxLineLen(FILE *tfd); */
char *strResize(char *ptr, size_t oldsize, size_t newsize);
lci_data *destroyNodes( lci_data *head); 
lci_data *createNode( lci_data *head, int64_t lineLen );
size_t getFileMaxLineLen(FILE *tfd);


#if defined(ini_read_c) || defined(ini_write_c)
enum ini_states {Start, BgnSp, CommEndW, SectEndW, SectEndD, EqW1, EqW2, ValPSP, ValW, ValFSP, DqmW, Bslsh, Error, Stop };
size_t strNullLen(const char *str);
struct lci_data *iniFSM(struct lci_data *data, const int *in, int32_t len);
char eescape(char c);

#ifdef ini_read_c
char unescape(char c);
#endif /* ini_read_c */
#ifdef ini_write_c
//static const char* komment = ";#";
#endif /*ini_write_c*/
#endif /* ini_read_c, ini_write_c*/



char     *lciGETtoStr( const char *section, const char *param, char *dest, size_t dstlen );
/*int      lciGETtoStrlen(const char *section, const char *param, ...); */
int8_t     lciGETtoInt8( const char *filename, const char *section, const char *param);
int16_t    lciGETtoInt16(const char *filename, const char *section, const char *param);
int32_t    lciGETtoInt32(const char *filename, const char *section, const char *param);
int64_t    lciGETtoInt64(const char *filename, const char *section, const char *param);

double   lciGETtoDlb(const char *filename, const char *section, const char *param);
float    lciGETtoFlt(const char *filename, const char *section, const char *param);
long int lciGETtoLng(const char *filename, const char *section, const char *param);




#endif /* INI_READ_H_INCLUDED */

