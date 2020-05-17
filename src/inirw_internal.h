#ifndef INI_READ_H_INCLUDED
#define INI_READ_H_INCLUDED
#include <stdint.h> /* int64_t*/
#include "lightconfini.h"
 
 
/*
typedef struct lcini_data {

    enum nodeState {lcini_EMPTY, lcini_READY, lcini_CONTINUE, lcini_MULTILINE, lcini_ERROR } nodeState;
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
    enum valueDraw {lcini_EMPTYVAL, lcini_SIMPLEVAL, lcini_MULTILINEVAL, lcini_DQUOTEDVAL} valueDraw;
    char *comment;
    int32_t  commentLen;
    int32_t  commentStartPos;
    char     commentSign;
    char *errorMsg;
    int32_t  errorMsgLen;

    struct lcini_data *next; 
} lcini_data;
*/

/*
struct lcini_data *iniReadOut(const char *filename); 
char *strResize(char *ptr, size_t oldsize, size_t newsize);
lcini_data *destroyNodes( lcini_data *head); 
lcini_data *createNode( lcini_data *head, int64_t lineLen );
size_t getFileMaxLineLen(FILE *tfd);
*/

#if defined(ini_read_c) || defined(ini_write_c)
enum ini_states {Start, BgnSp, CommEndW, SectEndW, SectEndD, EqW1, EqW2, ValPSP, ValW, ValFSP, DqmW, Bslsh, Error, Stop };
size_t strNullLen(const char *str);
struct lcini_data *iniFSM(struct lcini_data *data, const char *in, int32_t len);
int eescape(int c);
int isascalnum(int c); /* Check if input is ASCII Alpha-numeric */
int checkspace(int c);  /* Only for ASCII characters */



#ifdef ini_read_c
int unescape(int c);
#endif /* ini_read_c */
#ifdef ini_write_c
//static const char* komment = ";#";
#endif /*ini_write_c*/
#endif /* ini_read_c, ini_write_c*/


/*
char     *lciGETtoStr( const char *section, const char *param, char *dest, size_t dstlen );
int8_t     lciGETtoInt8( const char *filename, const char *section, const char *param);
int16_t    lciGETtoInt16(const char *filename, const char *section, const char *param);
int32_t    lciGETtoInt32(const char *filename, const char *section, const char *param);
int64_t    lciGETtoInt64(const char *filename, const char *section, const char *param);

double   lciGETtoDlb(const char *filename, const char *section, const char *param);
float    lciGETtoFlt(const char *filename, const char *section, const char *param);
long int lciGETtoLng(const char *filename, const char *section, const char *param);
*/



#endif /* INI_READ_H_INCLUDED */

