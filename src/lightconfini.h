#ifndef LIGHTCONFINI_H_INCLUDED
#define LIGHTCONFINI_H_INCLUDED
#include <stdint.h> /* int64_t*/
 
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


typedef struct lcini_retdata{
    char *value;
    int32_t vallen;
    char *error;
    int32_t errorlen;
} lcini_retdata;

/* Ha maga függvény van átpakolva, nevestül, testestül */
/* extern void (mylciniReadOutFunct)(int line, int linelen, char *section, int sectionlen, char *param, int paramlen, char *value, int valuelen, char *comment, int commentlen, char *error, int errorlen ) ; */
/* Ha csak egy fggvényre mutató ptr */
typedef void (*lcinimyReadFunc)(int line, int linelen, char *section, int sectionlen, char *param, int paramlen, char *value, int valuelen, char *comment, int commentlen, char *error, int errorlen ) ; 
extern lcinimyReadFunc mylciniReadOutFunct;


struct lcini_data *lciniReadOut(const char *filename); 
int lciniReadOutOwn(const char *filename);

char *lciniGet(const char *filename, const char *section, const char *parameter, int32_t bufflen);
lcini_retdata *lciniGet2(const char *filename, const char *section, const char *parameter);




 
char     *lciGETtoStr( const char *section, const char *param, char *dest, size_t dstlen );
/*int      lciGETtoStrlen(const char *section, const char *param, ...); */
int8_t     lciGETtoInt8( const char *filename, const char *section, const char *param);
int16_t    lciGETtoInt16(const char *filename, const char *section, const char *param);
int32_t    lciGETtoInt32(const char *filename, const char *section, const char *param);
int64_t    lciGETtoInt64(const char *filename, const char *section, const char *param);

double   lciGETtoDlb(const char *filename, const char *section, const char *param);
float    lciGETtoFlt(const char *filename, const char *section, const char *param);
long int lciGETtoLng(const char *filename, const char *section, const char *param);

char *lciniStrResize(char *ptr, size_t oldsize, size_t newsize);
lcini_data *lciniDestroyNodes( lcini_data *head); 
lcini_data *lciniCreateNode( lcini_data *head, int64_t lineLen );
size_t lciniFileMaxLineLen(FILE *tfd); 

#endif /* LIGHTCONFINI_H_INCLUDED */

