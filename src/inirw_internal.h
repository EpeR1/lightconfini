#ifndef INI_RWINT_H_INCLUDED
#define INI_RWINT_H_INCLUDED
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
    enum lcini_states {Start, BgnSp, CommEndW, SectEndW, SectEndD, EqW1, EqW2, ValPSP, ValW, ValFSP, DqmW, Bslsh, Error, Stop };
    size_t strNullLen(const uint8_t *str);
    struct lcini_data *iniFSM(struct lcini_data *data, const uint8_t *in, int32_t len);

    uint8_t isascalnum(uint8_t c); /* Check if input is ASCII Alpha-numeric */
    uint8_t checkspace(uint8_t c);  /* Only for ASCII characters */
    size_t strLcpy(uint8_t *dst, size_t dstlen, const uint8_t *src, size_t srclen);
    uint8_t *lciniStrResize(uint8_t *ptr, size_t oldsize, size_t newsize);
    uint8_t unescape(uint8_t c);

#ifdef ini_read_c
/* int unescape(int c); */
#endif /* ini_read_c */
#ifdef ini_write_c
      uint8_t eescape(uint8_t c);
#endif /*ini_write_c*/
#endif /* ini_read_c, ini_write_c*/

#endif /* INI_READ_H_INCLUDED */

