#ifndef LIGHTCONFINI_H_INCLUDED
#define LIGHTCONFINI_H_INCLUDED
#include <stdint.h> /* int64_t*/
 
typedef struct lcini_data {
    enum nodeState {lcini_EMPTY, lcini_READY, lcini_CONTINUE, lcini_MULTILINE, lcini_ERROR } nodeState;
    int32_t  lineNum;
    int32_t  lineLen;

    uint8_t *section;
    int32_t  sectionLen; 
    int32_t  sectionStartPos;
    uint8_t *param;
    int32_t  paramLen;
    int32_t  paramStartPos;
    uint8_t *value;
    int32_t  valueLen;
    int32_t  valueStartPos;
    enum valueDraw {lcini_EMPTYVAL, lcini_SIMPLEVAL, lcini_MULTILINEVAL, lcini_DQUOTEDVAL} valueDraw;
    uint8_t *comment;
    int32_t  commentLen;
    int32_t  commentStartPos;
    uint8_t  commentSign;
    uint8_t *errorMsg;
    int32_t  errorMsgLen;

    struct lcini_data *next; 
} lcini_data;


typedef struct lcini_shortret{
    uint8_t *ret;
    int32_t retlen;
    enum retType{lcini_shortretOK, lcini_shortretERROR, lcini_shortretEMPTY} retType;
} lcini_shortret;

/* Ha maga függvény van átpakolva, nevestül, testestül */
/* extern void (mylciniReadOutFunct)(int line, int linelen, char *section, int sectionlen, char *param, int paramlen, char *value, int valuelen, char *comment, int commentlen, char *error, int errorlen ) ; */
/* Ha csak egy fggvényre mutató ptr */
typedef void (*lcinimyReadFunc)(int line, int linelen, char *section, int sectionlen, char *param, int paramlen, char *value, int valuelen, char *comment, int commentlen, char *error, int errorlen ); 
extern lcinimyReadFunc mylciniReadOutFunct;


struct lcini_data *lciniReadOut(const char *filename); 
int lciniReadOutOwn(const char *filename);

lcini_data     *lciniGet(lcini_data *head, const char *section, const char *parameter); /* FETCH requested value TO an lcini_data object, FROM lcini_data list*/ 
char           *lciniGetStr(lcini_data *head, const char *section, const char *parameter, char *dst, int dstlen); /* FETCH requested value TO null-terminated-string, FROM lcini_data list */
lcini_shortret *lciniGetShort(lcini_data *head, const char *section, const char *parameter);    /* FETCH requested value TO shortret object, FROM lcini_data list*/
lcini_shortret *lciniGetFromFileShort(const char *filename, const char *section, const char *parameter); /* FETCH requested value TO shortret object FROM file */
char           *lciniGetFromFileStr(const char *filename, const char *section, const char *parameter, char *buff, int len);




lcini_data *lciniDestroyNodes( lcini_data *head); 
lcini_data *lciniCreateNode( lcini_data *head, int lineLen );
size_t lciniFileMaxLineLen(FILE *tfd); 

#endif /* LIGHTCONFINI_H_INCLUDED */

