/* INI fájl olvasás */

#include <stdio.h>
#include <string.h> /* strncpy + strerror */
#include <stdlib.h> /* malloc(), atoi(), exit(EXIT_FALIURE) */
#include <errno.h>  /* errno */
#include <stdarg.h>
#include <stdint.h> /* int64_t */

#define ini_read_c
#include "inirw_internal.h"
#include "lightconfini.h"



size_t strNullLen(const uint8_t *str){
    if(str == NULL){
        return 0;
    } else { 
        return strlen((const char*)str);
    } 
}


size_t strLcpy(uint8_t *dst, size_t dstlen, const uint8_t *src, size_t srclen){  /* Safe strncpy() */
    uint8_t *tdst=dst; 
    const uint8_t *tsrc=src;
    size_t i=dstlen, j=srclen;

    if(dst == NULL || src == NULL || dstlen == 0){
        return 0;
    } else {
        while(--i && j-- && *tsrc != '\0'){ /* [ --i => Strats from dstlen=2] [ j-- => Stops on n=srclen] */
            *(tdst++) = *(tsrc++);  /* dst[n] = src[n]; n++; */
        }
        *tdst = '\0'; /* Applies from dstlen=n=1 */
        return tdst-dst+1;   /* all copied bytes, incl. '\0' */
    }
}


uint8_t *lciniStrResize(uint8_t *ptr, size_t oldsize, size_t newsize){
    uint8_t *tmp=NULL;

    if(newsize <= 0){           /* deleting */
        free(ptr);
        return NULL;

    } else if(newsize != oldsize){       /* If any changes needed */
        tmp = (uint8_t *) malloc(newsize*sizeof(uint8_t));
        memset(tmp, 0, newsize*sizeof(uint8_t));
        if(tmp == NULL){            /* String is not changed at malloc error */
            return ptr;
        } else if(ptr == NULL) {
            return tmp;

        } else {   
            /*strncpy(tmp, ptr, newsize);*/ /* c89 */
            /*snprintf(tmp, newsize, "%s", ptr);*/ /* c99 */
            strLcpy(tmp, newsize, ptr, oldsize); /* c89, thread-safe */
            free(ptr);
            return tmp;
        } 
    } else {
        return ptr;
    }
}


uint8_t unescape(uint8_t c){
    if(c == 'n'){            /* Newline */
        return '\n';
    } else if(c == 'a'){     /* Bell */
        return '\a';
    } else if(c == 'b'){     /* Backspace */
        return '\b';
    } else if(c == 'f'){     /* Formfeed Page Break */
        return '\f';
    } else if(c == 'r'){     /* Carrige return */
        return '\r';
    } else if(c == 't'){     /* Horizontal tab */
        return '\t';
    } else if(c == 'v'){     /* Vertical tab */
        return '\v';
    } else if(c == 'e'){
        return 0x1B;
    /*} else if(c < 0x20){  // debug
        return '~';    */
    } else {                /* Original is OK */
        return c;  
    }
}

#ifdef ini_write_c
uint8_t eescape(uint8_t c){
    if(c == '\n'){          /* Newline */
        return 'n';
    } else if(c == '\a'){   /* Bell */
        return 'a';
    } else if(c == '\b'){   /* Backspace */
        return 'b';
    } else if(c == '\f'){   /* Formfeed Page Break */
        return 'f';
    } else if(c == '\r'){   /* Carrige return */
        return 'r';
    } else if(c == '\t'){   /* Horizontal tab */
        return 't';
    } else if(c == '\v'){   /* Vertical tab */
        return 'v';
    /*} else if(c < 0x20){  //debug
        return '~'; */
    } else {                /* Original is OK */
        return c;  
    }
}
#endif

uint8_t isascalnum(uint8_t c){ /* Check if input is ASCII Alpha-numeric */
    if( 0x30 <= c && c <= 0x39){    /* Numeric */
        return 1;
    } else if (0x41 <= c && c <= 0x5a){ /* UPPER */
        return 1;
    } else if( 0x61 <= c && c <= 0x7a){ /* lower */
        return 1;
    } else {
        return 0;
    }
}

uint8_t checkspace(uint8_t c){  /* Only for ASCII characters */
    switch (c) {
    case 0x20:      /* space (SPC) */
        return 1;
    case 0x09:      /* horizontal tab (TAB) */
        return 1;
    case 0x0A:      /* newline (LF) */
        return 1;
    case 0x0B:      /* vertical tab (VT) */
        return 1;
    case 0x0C:      /* feed (FF) */
        return 1;
    case 0x0d:      /* carrige return (CR) */
        return 1;
    default:
        return 0;
    }
}



size_t lciniFileMaxLineLen(FILE *tfd){
    int c=0;
    size_t i=0, max=0;
    if (tfd != NULL){
        while( c != EOF){
            c = fgetc(tfd);
            if (c == '\n' || c == EOF) {    /* Line End */
                i++;                        /* The "\n" is a character also */ 
                if(max < i){
                    max = i;
                }
                i = 0;                      /* Recount on every newline */
            } else {
                i++;
            }
        }
        fseek(tfd, 0, SEEK_SET);            /* Pos. to the start */
        return max;
    } else {                                /* File open error */
        return -1;
    }       
}

struct lcini_data *iniFSM(struct lcini_data *data, const uint8_t *in, int32_t len){

    int32_t i,j, vallen=len; 
    enum lcini_states pstate=Start, state=Start;
    /* char cc; */
    if(data == NULL){
        return NULL;
    } else {

        for(i=0, j=0; i<len; i++, j++){
            /*cc = in[i];*/  /*debug*/
            switch (state) { 
                case Start: 
                    if(data->nodeState == lcini_MULTILINE){                                               /* Bypass to the DQM collection */
                        state = DqmW;   
                        j = data->valueLen-2;       /* (x-1) => \0; (x-2) => j--; */
                        data->value = lciniStrResize(data->value, data->valueLen, data->valueLen+len+1);   /* lciniStrResize(ptr, oldsize, newsize */
                        vallen = data->valueLen+len+1;
                        data->nodeState = lcini_CONTINUE;
                        i--;                                    /* first char is collected also */
                        pstate = Start;
                        break;
                    } else if(in[i] == '\n' || in[i] == '\r' /*|| in[i] == '\0'*/){   /* Line End */
                        state = Stop;                   
                        i--;
                    }else if(in[i]==0xEF || in[i]==0xBB || in[i]==0xBF || in[i]==0xFF || in[i]==0x00 ){ /* UTF8, UTF16, UTF32 BOM */
                        state = Start;
                    } else if(checkspace(in[i])){                  /* ISSPACE, but not line end */
                        state = BgnSp;
                    } else if(in[i] == ';' || in[i] == '#' ){   /* Comment sign first */
                        j = -1;
                        state = CommEndW;
                        data->comment = lciniStrResize(data->comment, data->commentLen, len);
                        memset(data->comment, 0, len);
                        data->commentStartPos = i;
                        data->sectionStartPos = -1;             /* Comment only line */
                        data->commentSign = in[i];
                    } else if(in[i] == '['){                    /* Section start */
                        j = -1;
                        state = SectEndW;
                        data->section = lciniStrResize(data->section, data->sectionLen, len);
                        memset(data->section, 0, len);
                        data->sectionStartPos = i;              /* Brackets [] are counted! */
                    } else if(isascalnum(in[i]) ){                 /* Parameter is starting */
                        j = -1;
                        state = EqW1;    
                        data->param = lciniStrResize(data->param, data->paramLen, len);
                        memset(data->param, 0, len);
                        data->paramStartPos = i;
                        i--;
                    } else {                                    /* eg: Control characters in pos 1. */
                        state = Error; 
                        i--;
                    }
                    data->nodeState = lcini_EMPTY;
                    data->valueDraw = lcini_EMPTYVAL;
                    pstate = Start;
                    break;


                case BgnSp:                                     /* Space() at Begining */
                    if (in[i] == '\n' || in[i] == '\r' ){
                        state = Stop;
                        i--;
                    } else if (in[i] == ';' || in[i] == '#'){   /* Comment is coming */
                        j = -1;
                        state = CommEndW;
                        data->comment = lciniStrResize(data->comment, data->commentLen, len);
                        memset(data->comment, 0, len);
                        data->commentStartPos = i;
                        data->sectionStartPos = -1;             /* No Section started in this line */
                        data->commentSign = in[i];
                    } else if(in[i] == '['){                    /* Section is starting */
                        j = -1;
                        state = SectEndW;
                        data->section = lciniStrResize(data->section, data->sectionLen, len);
                        memset(data->section, 0, len);
                        data->sectionStartPos = i;
                    } else if (isascalnum(in[i])){                 /* Parameter will be */
                        j = -1;
                        state = EqW1;
                        data->param = lciniStrResize(data->param, data->paramLen, len);
                        memset(data->param, 0, len);
                        data->paramStartPos = i;
                        i--;
                    } else if(checkspace(in[i]) /*|| in[i] == '\0' */ ){
                        state = BgnSp;
                    } else {
                        state = Error;
                        i--;
                    }
                    pstate = BgnSp;
                    break;
 

                case CommEndW:      /* Till comment to the line end */
                    if(in[i] == '\n' || in[i] == '\r'){
                        state = Stop;
                        data->nodeState = lcini_READY;
                        data->comment[j] = '\0';
                        data->commentLen = j+1;
                        i--;
                    } else {
                        data->comment[j] = in[i];
                    }
                    pstate = CommEndW;
                    data->commentLen = j+1;
                    break;        


                case SectEndW:              /* Section Collector */
                    if(in[i] == ']'){
                        state = SectEndD;
                        data->section[j] = '\0';
                        data->sectionLen = j+1;
                        if(j == 0){         /* empty section */
                            state = Error;
                        }
                    } else if(isascalnum(in[i]) || in[i]=='/' || in[i]==' ' || in[i]=='_' || in[i]=='-' || in[i]=='.'){   
                        data->section[j] = in[i];
                    } else /*if((in[i] >= 0x00 && in[i] < 0x20) || in[i] == ';' || in[i] == '#' )*/ {       /* túl korai sorvég, SP(), vagy komment */
                        state = Error;
                        data->section[j] = '\0';
                        data->sectionLen = j+1;
                        i--;
                    } /*else {
                        data->section[j] = in[i];
                    }*/
                    pstate = SectEndW;
                    break;


                case SectEndD:          /* Section collected, then: SP(), line_end, or comment */
                    if(in[i]=='\n' || in[i]=='\r'){
                        state = Stop;
                        data->nodeState = lcini_READY;
                        i--;
                    }else if(checkspace(in[i])){
                        state = SectEndD;             /* remain here */
                    } else if (in[i] == ';' || in[i] == '#'){
                        j = -1;
                        state = CommEndW;
                        data->comment = lciniStrResize(data->comment, data->commentLen, len);
                        memset(data->comment, 0, len*sizeof(uint8_t));
                        data->commentStartPos = i;
                        data->commentSign = in[i];
                        data->nodeState = lcini_CONTINUE;
                    } else {
                        state = Error;      /* wrong character in line */
                        data->sectionLen = j+1;
                        i--;
                    }
                    pstate = SectEndD;
                    break;


                case EqW1:              /* Parameter name, befora EQ_sign(=) */
                    if(in[i] == '='){    /* EQ sign arrived */
                        state = ValPSP;
                        data->param[j] = '\0';
                        data->paramLen = j + 1;
                    } else if(isascalnum(in[i]) || in[i]=='[' || in[i]==']' || in[i]=='_' || in[i]=='-' || in[i]=='.'){   /* Arrays are supported ins parameter name, like in PHP */
                        data->param[j] = in[i];
                    } else if(checkspace(in[i]) && in[i] != '\r' && in[i] != '\n'){   /* Only SPACE allowed between Param_name and EQ_sign */
                        state = EqW2;
                        data->param[j] = '\0';
                        data->paramLen = j + 1;
                    } else {
                        state = Error;
                        data->paramLen = j+1;
                        i--;
                    }
                    pstate = EqW1;
                    break;


                case EqW2:              /* SPACE before EQ_sign */
                    if(in[i] == '='){    /* EQ_sign arrived */
                        j = -1;
                        state = ValPSP;
                    } else if(checkspace(in[i])){   /* Another space, remain here */
                        state = EqW2;
                    } else {
                        state = Error;
                        i--;
                    }
                    pstate = EqW2;
                    break;


                case ValPSP:    /* After EQ_sign,  (SPACE, or value can arrive) */
                    if(in[i] == '\n' || in[i] == '\r' /*|| in[i] == '\0'*/ ){   /* Empty value */
                        state = Stop;
                        data->nodeState = lcini_READY;
                        data->valueDraw = lcini_SIMPLEVAL;
                        i--;
                    } else if(in[i] == ';' || in[i] == '#'){  /* Comment */
                        j = -1;
                        state = CommEndW;
                        data->comment = lciniStrResize(data->comment, data->commentLen, len);
                        memset(data->comment, 0, len);
                        data->commentStartPos = i;
                        data->nodeState = lcini_CONTINUE;
                        data->valueDraw = lcini_SIMPLEVAL;
                        data->commentSign = in[i];

                    } else if( pstate != Bslsh && in[i] == '\"' ){   /* DQM arrived => DQM collector */
                        j = -1;
                        state = DqmW;
                        data->value = lciniStrResize(data->value, data->valueLen, len);
                        memset(data->value, 0, len);
                        data->valueStartPos = i;
                        data->nodeState = lcini_CONTINUE;
                        data->valueDraw = lcini_DQUOTEDVAL;
                    } else if(checkspace(in[i])){                      /* Another SP(), remain here */
                        state = ValPSP;              
                    } else if(isascalnum(in[i]) || in[i]=='-') {       /* Normal_Value collector */
                        j = -1;
                        state = ValW;
                        data->value = lciniStrResize(data->value, data->valueLen, len);
                        memset(data->value, 0, len);
                        data->valueStartPos = i;
                        data->nodeState = lcini_CONTINUE;
                        data->valueDraw = lcini_SIMPLEVAL;
                        i--;
                    } else {
                        state = Error;
                        i--;
                    }
                    pstate = ValPSP;
                    break;


                case ValW:   /* Normal_Value collector */
                    if(in[i] == '\n' || in[i] == '\r'){   /* end -> new_line */
                        state = Stop;
                        data->nodeState = lcini_READY;
                        data->value[j] = '\0';
                        data->valueLen = j + 1;
                        i--;
                    } else if(in[i] == ';' || in[i] == '#'){   /* comment */
                        data->nodeState = lcini_CONTINUE;
                        data->value[j] = '\0';
                        data->valueLen = j+1;
                        state = CommEndW;
                        j = -1;
                        data->comment = lciniStrResize(data->comment, data->commentLen, len);
                        memset(data->comment, 0, len);
                        data->commentStartPos = i;
                        data->commentSign = in[i];
                        data->nodeState = lcini_CONTINUE;
                    /*
                        } else if( in[i] == '\\' ){  // Backslash support 
                            j--;            //A '\' nem számít bele!
                            state = Bslsh;   
                    */
                    } else if(isascalnum(in[i]) || in[i]=='_' || in[i]=='-' || in[i]=='.' ){ /* Regular characters without SP() */
                        data->value[j] = in[i];
                    } else if(checkspace(in[i])){            /* SPACE arrived -> line_end */
                        data->value[j] = '\0';
                        data->valueLen = j+1;
                        data->nodeState = lcini_READY;
                        state = ValFSP;
                        i--;
                    } else {
                        state = Error;
                        data->valueLen = j+1;
                        i--;
                    }
                    pstate = ValW;
                    break;


                case Bslsh: /* Backslash -> Escaping needed */
                    if(pstate == ValW){
                        state = ValW;
                        data->value[j] = unescape(in[i]);
                    } else if(pstate == SectEndW){
                        state = SectEndW;
                        data->section[j] = unescape(in[i]);
                    }else if(pstate == DqmW){
                        state = DqmW;
                        data->value[j] = unescape(in[i]);
                        if(in[i] == '\n'){      /* Workaround for UNIX line end */
                            i--;
                        }
                        if( in[i] == '\r'){     /* Workaround for WIN line endings */
                            j--;
                        }
                    } else {
                        state = Error;
                        i--;
                    }
                    /* if(0x00 < in[i] && in[i] < 0x20){   //láthatatlan: pl \\n (escapelt sorvége) //Vagy inkább mégis kell?
                        j--;
                    }     */
                    pstate = Bslsh;
                    break;


                case ValFSP:    /* Space after value */
                    if( in[i] == '\n' || in[i] == '\r'){
                        state = Stop;
                        i--;
                    } else if( in[i] == ';' || in[i] == '#'){
                        j = -1;
                        state = CommEndW;
                        data->comment = lciniStrResize(data->comment, data->commentLen, len);
                        memset(data->comment, 0, len);
                        data->commentStartPos = i;
                        data->commentSign = in[i];
                        data->nodeState = lcini_CONTINUE;
                    } else if(checkspace(in[i])){
                        state = ValFSP;         /* SP() -> ermain here */ 
                    } else {
                        state = Error;
                        i--;
                    }
                    pstate = ValFSP;
                    break;


                case DqmW:   /* DQM Collector (Double Quotation MArk) */
                    if( in[i] == '\"'){        /* Second DQM => Sring_end */
                        data->value[j] = '\0';
                        data->valueLen = j+1;
                        data->nodeState = lcini_READY;
                        state = ValFSP;
                    } else if ( in[i] == '\\'){    /* Backslash */
                        j--;
                        state = Bslsh;
                    } else if(len > 1 && in[i] == '\n' && in[i-1] == '\\'){       /* UNIX style line endings*/
                        data->value[j] = '\0';                              /* '\\n' => '\n\0' */
                        data->valueLen = j+1;
                        data->nodeState = lcini_MULTILINE;
                        data->valueDraw = lcini_MULTILINEVAL;
                        state = Stop;
                        i--;
                    } else if(len > 2 && in[i] == '\n' && in[i-1] == '\r' && in[i-2] == '\\'){ /* WINDOWS style */
                        data->value[j]   = '\n';
                        data->value[j+1] = '\0';
                        data->valueLen = j+2;
                        data->nodeState = lcini_MULTILINE;
                        data->valueDraw = lcini_MULTILINEVAL;
                        state = Stop;
                        i--;
                    } else if( pstate != Bslsh && (in[i] == '\r' || in[i] == '\n' || in[i] == '\0')){ /* Too early Line_end */
                        state = Error;
                        data->valueLen = j+1;
                        i--;
                    } else {
                        data->value[j] = in[i];
                    }
                    pstate = DqmW;
                    break; 


                case Error:
                    data->errorMsg = lciniStrResize(data->errorMsg, data->errorMsgLen, 256);  /* Returns zero-filled string */

                    if(pstate == SectEndW || pstate == SectEndD){
                        data->errorMsgLen = sprintf((char*)data->errorMsg, "Illegal character or EMPTY SECTION! (line: %d, pos: %d)", data->lineNum, i+1) +1;
                    } else if(pstate == EqW1 || pstate == EqW2){
                        data->errorMsgLen = sprintf((char*)data->errorMsg, "Illegal character at PARAMETER! (line: %d, pos: %d)", data->lineNum, i+1) +1;
                    } else if(pstate == ValPSP || pstate == ValW || pstate == ValFSP ){
                        data->errorMsgLen = sprintf((char*)data->errorMsg, "Illegal character at VALUE! (line: %d, pos: %d)", data->lineNum, i+1) +1;
                    } else if(pstate == DqmW){
                        data->errorMsgLen = sprintf((char*)data->errorMsg, "Double quotation mark needed! (line: %d, pos: %d)", data->lineNum, i+1) +1;
                    } else {    
                        data->errorMsgLen = sprintf((char*)data->errorMsg, "Illegal character! (line: %d, pos: %d)", data->lineNum, i+1) +1;
                    }

                    i--;
                    state = Stop;    
                    data->nodeState = lcini_ERROR;
                    pstate = Error;
                    break;
 
                case Stop:
                    if(in[i] == '\n' || in[i] == '\r' || in[i] == '\0' || pstate == Error){   /* Line Ended */
                        state = Stop;
                        if(strNullLen(data->section) == 0){data->sectionLen = 0;}
                        if(strNullLen(data->param) == 0){data->paramLen = 0;}
                        if(strNullLen(data->value) == 0){data->valueLen = 0;}
                        if(strNullLen(data->comment) == 0){data->commentLen = 0;}
                        if(strNullLen(data->errorMsg) == 0){data->errorMsgLen = 0;}
                        data->section = lciniStrResize(data->section, len, data->sectionLen);
                        data->param = lciniStrResize(data->param, len, data->paramLen);
                        data->value = lciniStrResize(data->value, vallen, data->valueLen);
                        data->comment = lciniStrResize(data->comment, len, data->commentLen);
                        data->errorMsg = lciniStrResize(data->errorMsg, 256, data->errorMsgLen);
                        if(data->sectionStartPos < 0 && data->param==NULL && data->value==NULL && data->comment==NULL && data->errorMsg==NULL){
                            data->nodeState = lcini_EMPTY;
                            data->valueDraw = lcini_EMPTYVAL;
                        }
                        return data;
                    } else {    /* Everything else -> ERROR */
                        state = Error;
                    }
                    pstate = Stop;
                    break;

                default:
                    state = Start;
                    break;
            }
        }
        return data;
    }
}




lcini_data *lciniCreateNode( lcini_data *head, int lineLen ){ /* Creates one Node */
    lcini_data *curr;

    curr = (lcini_data *) calloc(1, sizeof(lcini_data));
    curr->nodeState = lcini_EMPTY; 
    curr->lineNum = 0;
    curr->lineLen = lineLen;

    if(lineLen > 0){
        curr->section  = (uint8_t *) calloc(lineLen, sizeof(uint8_t));
        curr->param    = (uint8_t *) calloc(lineLen, sizeof(uint8_t));
        curr->value    = (uint8_t *) calloc(lineLen, sizeof(uint8_t));
        curr->comment  = (uint8_t *) calloc(lineLen, sizeof(uint8_t));
        curr->errorMsg = (uint8_t *) calloc(lineLen, sizeof(uint8_t));
    } else {
        curr->section = NULL;
        curr->param = NULL;
        curr->value = NULL;
        curr->comment = NULL;
        curr->errorMsg = NULL;
        lineLen = 0;
    }
    curr->sectionLen = lineLen;
    curr->sectionStartPos = -1;
    curr->paramLen = lineLen;
    curr->paramStartPos = -1;
    curr->valueLen = lineLen;
    curr->valueStartPos = -1;
    curr->valueDraw = lcini_EMPTYVAL;
    curr->commentLen = lineLen;
    curr->commentStartPos = -1;
    curr->commentSign = '\0';
    curr->errorMsgLen=lineLen;
    curr->next = NULL;

    if(head != NULL){
        head->next = curr;
    }
    return curr;
}        


lcini_data *lciniDestroyNodes( lcini_data *head){ /* Destroys Nodes from HEAD to the end */
    lcini_data *tmp, *node=head;
        while(node != NULL){
            free(node->section);
            free(node->param);
            free(node->value);
            free(node->comment);
            free(node->errorMsg);
            tmp = node;
            node = node->next;
            free(tmp);
        }
    return NULL;
}


lcini_shortret *lciniMKShortRet(int bufflen){
    lcini_shortret *dt=NULL;

    dt = (lcini_shortret *) calloc(1, sizeof(lcini_shortret));
    if(dt){
        dt->ret = (char *) calloc(bufflen, sizeof(char));
        dt->retlen = bufflen;
        dt->retType = lcini_shortretEMPTY;
    }
    return dt;
}


void lciniDestroyShortRet(lcini_shortret *dt){
    if(dt){
        free(dt->ret);
    }
    free(dt);
}


struct lcini_data *lciniReadOut(const char *filename){      /* Reads the entire file to a linked-list */

    int  c=0; 
    uint8_t *buff; 
    FILE *fp=NULL;
    int32_t linemax, line=0, pos=0;
    struct lcini_data *prev=NULL, *curr=NULL, *list=NULL;
    /*char cc;*/

    fp = fopen(filename, "rb"); 
    if(!fp ){   
        list = lciniCreateNode(NULL, 0);
        list->errorMsg = lciniStrResize(list->errorMsg, 0, 256);
        list->errorMsgLen = sprintf((char*)list->errorMsg, "File opening error. Errno: %d (%s)", errno, strerror(errno))+1;
        list->errorMsg = lciniStrResize(list->errorMsg, 256, list->errorMsgLen);
        list->nodeState = lcini_FLERROR;
    
    } else {
        linemax = lciniFileMaxLineLen(fp) +3;   /* EOF => "\n\n\0" */
        buff = (uint8_t*)malloc(linemax*sizeof(uint8_t));
        memset(buff, 0, linemax*sizeof(uint8_t));

        while( c != EOF){
            c = fgetc(fp);
            /*cc = c;*/     /* debug */

            if( c == '\n' || c == EOF){   

                line++;
                buff[pos] = '\n';

                if(curr == NULL || curr->nodeState != lcini_MULTILINE ){
                    curr = lciniCreateNode(NULL, linemax);
                }
                if(list == NULL){   /* First node */
                    list = curr;
                }
                if(prev && curr && prev != curr && prev->section ){ /* Copy SECTION string from previous node to current */
                    curr->section = lciniStrResize(curr->section, curr->sectionLen, prev->sectionLen);
                    curr->sectionLen = prev->sectionLen;
                    strLcpy(curr->section, curr->sectionLen, prev->section, prev->sectionLen);
                    /* memcpy(curr->section, prev->section, prev->sectionLen); */
                }
                if(curr){      /* Call the Finite-State-Machine processor */
                    curr->lineNum = line;
                    curr->lineLen = pos + 1;
                    curr = iniFSM(curr, buff, linemax);
                }
                if(curr->nodeState == lcini_EMPTY){   /* Dropping empty lines */
                    if(list == curr || prev == curr ){   /* File contains empty lines */
                        list = NULL;
                        prev = NULL;
                    }
                    curr = lciniDestroyNodes(curr);
                }

                if(!prev){ /* Prew is null */
                    prev = curr;
                } else {    /* Register current for next step */
                    prev->next = curr;
                    if(prev->next && curr->nodeState != lcini_MULTILINE){  /* Step only, when current is not multiline */
                        prev = prev->next;
                    }
                }
                
                if(curr && curr->nodeState == lcini_ERROR){   /* Stop on first ERROR */
                    /* return list; */
                }
                if(c == EOF && curr && curr->nodeState == lcini_MULTILINE){
                    curr->nodeState = lcini_ERROR;
                    curr->errorMsg = lciniStrResize(curr->errorMsg, curr->errorMsgLen, 45);
                    curr->errorMsgLen = sprintf((char*)curr->errorMsg, "File ended without double quotation mark!") +1;
                }
                pos = 0;
                memset(buff, 0, linemax*sizeof(uint8_t));
            } else {
                buff[pos] = c;
                pos++;
            }
        }
        free(buff);
    }

    if(fp){
        fclose(fp);
    }
    return list;
}
 


int lciniReadOutOwn(const char *filename){      /* Reads the entire file to a linked-list */

    int  c=0; 
    uint8_t *buff=NULL; 
    FILE *fp=NULL;
    int32_t linemax, line=0, pos=0;
    struct lcini_data *curr;

    curr = lciniCreateNode(NULL,0);
    /* buff = (uint8_t*) calloc(256, sizeof(uint8_t)); */
    fp = fopen(filename, "rb"); 
    
    if(!fp){   /* fp == NULL */
        if( mylciniReadOutFunct != NULL){
            buff = lciniStrResize(buff, 0, 256);
            pos = sprintf((char*)buff, "File opening error. Errno: %d (%s)", errno, strerror(errno)) +1;
            /* buff = lciniStrResize(buff, 256, pos); */
            (*mylciniReadOutFunct)(0,0, NULL,0, NULL,0, NULL,0, NULL,0, (char*)buff, pos);
        }
    
    } else {
        linemax = lciniFileMaxLineLen(fp) +1; 
        buff = lciniStrResize(buff, 0, linemax);

        while( c != EOF){
            c = fgetc(fp);

            if( c == '\n' || c == EOF){   
                line++;
                buff[pos] = '\n';

                if(curr){      /* Call the Finite-State-Machine processor */
                    curr->lineNum = line;
                    curr->lineLen = pos + 1;
                    iniFSM(curr, buff, linemax);
                } 
                if(c == EOF && curr && curr->nodeState == lcini_MULTILINE){
                    curr->nodeState = lcini_ERROR;
                    curr->errorMsg = lciniStrResize(curr->errorMsg, curr->errorMsgLen, 45);
                    curr->errorMsgLen = sprintf((char*)curr->errorMsg, "File ended without double quotation mark!")+1;
                }
                if(curr->nodeState != lcini_EMPTY && curr->nodeState != lcini_MULTILINE && mylciniReadOutFunct != NULL ){   /* Call with function ptr */
                    (*mylciniReadOutFunct)(line, pos+1, (char*)curr->section, curr->sectionLen, (char*)curr->param, curr->paramLen, (char*)curr->value, curr->valueLen, (char*)curr->comment, curr->commentLen, (char*)curr->errorMsg, curr->errorMsgLen);
                }
                if(curr->nodeState != lcini_MULTILINE){
                    curr->paramLen = 0;
                    curr->valueLen = 0;
                    curr->commentLen = 0;
                    curr->errorMsgLen = 0;
                }
                pos = 0;
                memset(buff, 0, linemax*sizeof(uint8_t));
            } else {
                buff[pos] = c;
                pos++;
            }
        }
    }

    if(fp){
        fclose(fp);
    }
    lciniDestroyNodes(curr);
    free(buff);
    return line;
}



lcini_data *lciniGet(lcini_data *head, const char *section, const char *parameter){ /* Retuires null-terminated string */

    lcini_data *curr=head, *ret=NULL;
    if(head && head->nodeState == lcini_FLERROR){
        ret = head;
    } else if(parameter){   /* parameter != NULL */
        while(curr != NULL){
            if((!section && !curr->section) || (section && curr->section && !strcmp(section, (char*)curr->section)) ){
                if(curr->param && !strcmp(parameter, (char*)curr->param) ){
                    ret = curr;
                    /* break; */    /* for first hit */
                }
            } 
            curr = curr->next;
        }
    }
    return ret;
}



int lciniGetStr(lcini_data *head, const char *section, const char *parameter, char *dst, int dstlen){ /* Retuires null-terminated string */
    
    int r=0;
    lcini_data *curr=NULL;

    curr = lciniGet(head, section, parameter);
    if(curr){ /* curr !== NULL */
        r = strLcpy((uint8_t*)dst, dstlen, curr->value, curr->valueLen);
        if(curr->nodeState == lcini_ERROR || curr->nodeState == lcini_FLERROR){
            if(!r){
                r++;
            }
            r = 0-r;
        }
    }
    return r;
}



lcini_shortret *lciniGetShort(lcini_data *head, const char *section, const char *parameter, lcini_shortret *ret){ /* Retuires null-terminated string */
    
    lcini_shortret *dt=NULL;
    lcini_data *curr=NULL;
    
    curr = lciniGet(head, section, parameter);
    if(!ret){
        dt = lciniMKShortRet(1);
    } else {
        dt = ret;
    }
    if(curr && dt){ /* curr !== NULL */
        if(curr->nodeState == lcini_ERROR || curr->nodeState == lcini_FLERROR){
            dt->ret = (char *) lciniStrResize((uint8_t *)dt->ret, dt->retlen, curr->errorMsgLen);
            dt->retlen = strLcpy((uint8_t*)dt->ret, curr->errorMsgLen, curr->errorMsg, curr->errorMsgLen);
            dt->retType = lcini_shortretERROR;
        } else {
            dt->ret = (char *) lciniStrResize((uint8_t *)dt->ret, dt->retlen, curr->valueLen);
            dt->retlen = strLcpy((uint8_t*)dt->ret, curr->valueLen, curr->value, curr->valueLen);
            dt->retType = lcini_shortretOK;
        }
    }
    return dt;
}



lcini_shortret *lciniGetFromFileShort(const char *filename, const char *section, const char *parameter, lcini_shortret *ret){      /* Reads the entire file to a linked-list */

    int  c=0; 
    uint8_t *buff=NULL; 
    FILE *fp=NULL;
    int32_t linemax, line=0, pos=0;
    lcini_data *curr;
    lcini_shortret *dt=NULL;

    curr = lciniCreateNode(NULL,0);
    fp = fopen(filename, "rb"); 

    if(!ret){
        dt = lciniMKShortRet(1);
    } else {
        dt = ret;
    }
        
    if(!fp){   /* fp == NULL */
        dt->ret = (char *)lciniStrResize((uint8_t *)dt->ret, dt->retlen, 256);
        dt->retlen = sprintf(dt->ret, "File opening error. Errno: %d (%s)", errno, strerror(errno)) +1;
        dt->retType = lcini_shortretERROR;

    } else {
        linemax = lciniFileMaxLineLen(fp) +1; 
        buff = lciniStrResize(buff, 0, linemax);

        while(c != EOF){
            c = fgetc(fp);

            if( c == '\n' || c == EOF){   
                line++;
                buff[pos] = '\n';

                if(curr){      /* Call the Finite-State-Machine processor */
                    curr->lineNum = line;
                    curr->lineLen = pos + 1;
                    iniFSM(curr, buff, linemax);
                } 
                if(c == EOF && curr && curr->nodeState == lcini_MULTILINE){
                    curr->nodeState = lcini_ERROR;
                    curr->errorMsg = lciniStrResize(curr->errorMsg, curr->errorMsgLen, 45);
                    curr->errorMsgLen = sprintf((char*)curr->errorMsg, "File ended without double quotation mark!")+1;
                }
                /* Search for section && parameter */
                if(curr->nodeState != lcini_EMPTY && curr->nodeState != lcini_MULTILINE ){
                    if ((!section && !curr->section) || (section && curr->section && !strcmp(section, (char*)curr->section)) ){
                        if( parameter && curr->param && !strcmp(parameter, (char*)curr->param) ){

                            if(curr->nodeState == lcini_ERROR){
                                dt->ret = (char *) lciniStrResize((uint8_t *)dt->ret, dt->retlen, curr->errorMsgLen);
                                dt->retlen = strLcpy((uint8_t *)dt->ret, curr->errorMsgLen, curr->errorMsg, curr->errorMsgLen);
                                dt->retType = lcini_shortretERROR;
                            } else {
                                dt->ret = (char *) lciniStrResize((uint8_t *)dt->ret, dt->retlen, curr->valueLen);
                                dt->retlen = strLcpy((uint8_t *)dt->ret, curr->valueLen, curr->value, curr->valueLen);
                                dt->retType = lcini_shortretOK;
                            }
                        }
                    }
                }

                if(curr->nodeState != lcini_MULTILINE){
                    curr->paramLen = 0;
                    curr->valueLen = 0;
                    curr->commentLen = 0;
                    curr->errorMsgLen = 0;
                }
                pos = 0; 
                memset(buff, 0, linemax*sizeof(uint8_t));
            } else {
                buff[pos] = c;
                pos++;
            }
        }
    }

    if(fp){
        fclose(fp);
    }
    /* lciniDestroyShortRet(dt); */ 
    lciniDestroyNodes(curr);
    free(buff);
    return dt;
}


int lciniGetFromFileStr(const char *filename, const char *section, const char *parameter, char *dst, int dstlen){
        
    int r=0;
    lcini_shortret *scr=NULL;

    scr = lciniGetFromFileShort(filename, section, parameter, NULL);
    if(scr){ 
        if(scr->retType == lcini_shortretOK ){
            r = strLcpy((uint8_t*)dst, dstlen, (uint8_t*)scr->ret, scr->retlen);
        } else if(scr->retType == lcini_shortretERROR){
            r = 0-(scr->retlen);
        } 
    }
    return r;
}


char *lciniGetVersionStr(void){
#ifdef GIT_LAST
    char *ret = GIT_LAST;
# else 
    char *ret = "";
#endif
    return ret;
}