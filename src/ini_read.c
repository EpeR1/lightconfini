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

 
/*
extern void (mylciniReadOutFunct)(int line, int linelen, char *section, int sectionlen, char *param, int paramlen, char *value, int valuelen, char *comment, int commentlen, char *error, int errorlen );
*/


size_t strNullLen(const char *str){
    if(str == NULL){
        return 0;
    } else { 
        return strlen(str);
    } 
}

char *lciniStrResize(char *ptr, size_t oldsize, size_t newsize){
    char *tmp;

    if(newsize <= 0){           /* deleting */
        free(ptr);
        return NULL;

    } else if(newsize != oldsize){       /* If any changes needed */
        tmp = (char *) malloc(newsize*sizeof(char));
        memset(tmp, 0, newsize*sizeof(char));
        if(tmp == NULL){            /* String is not changed at malloc error */
            return ptr;
        } else if(ptr == NULL) {
            return tmp;

        } else {   
            strncpy(tmp, ptr, newsize); /* c89 */
            /*snprintf(tmp, newsize, "%s", ptr);*/ /* c99 */
            free(ptr);
            return tmp;
        } 
    } else {
        return ptr;
    }
}

int unescape(int c){
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

int eescape(int c){
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
        return '~';*/
    } else {                /* Original is OK */
        return c;  
    }
}


int isascalnum(int c){ /* Check if input is ASCII Alpha-numeric */

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

int checkspace(int c){  /* Only for ASCII characters */

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

    size_t c=0;
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

struct lcini_data *iniFSM(struct lcini_data *data, const char *in, int32_t len){

    int32_t i,j, vallen=len; 
    enum ini_states pstate=Start, state=Start;
    /*char cc; */

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
                    }else if((unsigned char)in[i] == 0xEF || (unsigned char)in[i] == 0xBB || (unsigned char)in[i] == 0xBF || (unsigned char)in[i] == 0xFF || (unsigned char)in[i] == 0x00 ){ /* UTF8, UTF16, UTF32 BOM */
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
                    if(in[i] == '\n' || in[i] == '\r'){
                        state = Stop;
                        data->nodeState = lcini_READY;
                        i--;
                    }else if(checkspace(in[i])){
                        state = SectEndD;             /* remain here */
                    } else if (in[i] == ';' || in[i] == '#'){
                        j = -1;
                        state = CommEndW;
                        data->comment = lciniStrResize(data->comment, data->commentLen, len);
                        memset(data->comment, 0, len);
                        data->commentStartPos = i;
                        data->commentSign = in[i];
                        data->nodeState = lcini_CONTINUE;
                    } else {
                        state = Error;      /* wrong character in line */
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
                    /* } else if( in[i] == '\\' ){  // Backslash support 
                    //    j--;            //A '\' nem számít bele!
                    //    state = Bslsh;   */
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
                    /* if(0x00 < in[i] && in[i] < 0x20){   //láthatatlan: pl \\n (escapelt sorvége)
                    //    j--;
                    //}     */
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
                        i--;
                    } else {
                        data->value[j] = in[i];
                    }
                    pstate = DqmW;
                    break; 


                case Error:
                    data->errorMsg = lciniStrResize(data->errorMsg, data->errorMsgLen, 256);  /* Returns zero-filled string */

                    if(pstate == SectEndW || pstate == SectEndD){
                        data->errorMsgLen = sprintf(data->errorMsg, "Illegal character or EMPTY SECTION! (line: %d, pos: %d)", data->lineNum, i+1) +1;
                    } else if(pstate == EqW1 || pstate == EqW2){
                        data->errorMsgLen = sprintf(data->errorMsg, "Illegal character at PARAMETER! (line: %d, pos: %d)", data->lineNum, i+1) +1;
                    } else if(pstate == ValPSP || pstate == ValW || pstate == ValFSP ){
                        data->errorMsgLen = sprintf(data->errorMsg, "Illegal character at VALUE! (line: %d, pos: %d)", data->lineNum, i+1) +1;
                    } else if(pstate == DqmW){
                        data->errorMsgLen = sprintf(data->errorMsg, "Double quotation mark needed! (line: %d, pos: %d)", data->lineNum, i+1) +1;
                    } else {    
                        data->errorMsgLen = sprintf(data->errorMsg, "Illegal character! (line: %d, pos: %d)", data->lineNum, i+1) +1;
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




lcini_data *lciniCreateNode( lcini_data *head, int64_t lineLen ){ /* Creates one Node */
    lcini_data *curr;

    curr = (lcini_data *) calloc(1, sizeof(lcini_data));
    curr->nodeState = lcini_EMPTY; 
    curr->lineNum = 0;
    curr->lineLen = lineLen;

    if(lineLen > 0){
        curr->section  = (char *) calloc(lineLen, sizeof(char));
        curr->param    = (char *) calloc(lineLen, sizeof(char));
        curr->value    = (char *) calloc(lineLen, sizeof(char));
        curr->comment  = (char *) calloc(lineLen, sizeof(char));
        curr->errorMsg = (char *) calloc(lineLen, sizeof(char));
    } else {
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



struct lcini_data *lciniReadOut(const char *filename){      /* Reads the entire file to a linked-list */

    int  c=0; 
    char *buff; 
    FILE *fp=NULL;
    int64_t linemax, line=0, pos=0;
    /*char cc;*/
    struct lcini_data *prev=NULL, *curr=NULL, *list = NULL;

    fp = fopen(filename, "rb"); 
    if(!fp ){   /* fp == NULL */
        list = lciniCreateNode(NULL, 256);
        list->errorMsg = lciniStrResize(list->errorMsg, list->errorMsgLen, 256);
        list->errorMsgLen = sprintf(list->errorMsg, "File opening error. Errno: %d (%s)", errno, strerror(errno) );
        list->nodeState = lcini_ERROR;
    
    } else {
        linemax = lciniFileMaxLineLen(fp) +1; 
        buff = (char *) malloc(linemax*sizeof(char));
        memset(buff, 0, linemax*sizeof(char));

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
                    memcpy(curr->section, prev->section, prev->sectionLen);    
                    curr->sectionLen = prev->sectionLen;
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
                pos = 0;
                memset(buff, 0, linemax*sizeof(char));
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
    char *buff=NULL; 
    FILE *fp=NULL;
    int64_t linemax, line=0, pos=0;
    struct lcini_data curr;
    /* char cc;*/

    curr.section = NULL;
    curr.param = NULL;
    curr.value = NULL;
    curr.comment = NULL;
    curr.errorMsg = NULL;
    buff = (char *) malloc(256*sizeof(char));
    memset(buff, 0, 256*sizeof(char));
    fp = fopen(filename, "rb"); 
    
    if(!fp && mylciniReadOutFunct != NULL){   /* fp == NULL */

        sprintf(buff, "File opening error. Errno: %d (%s)", errno, strerror(errno) );
        mylciniReadOutFunct(0,0, NULL,0, NULL,0, NULL,0, NULL,0, buff, 256);
    
    } else {
        linemax = lciniFileMaxLineLen(fp) +1; 
        buff = lciniStrResize(buff, 256, linemax);

        while( c != EOF){
            c = fgetc(fp);
            /*cc = c;*/     /* debug */

            if( c == '\n' || c == EOF){   
                line++;
                buff[pos] = '\n';


                if(1){      /* Call the Finite-State-Machine processor */
                    curr.lineNum = line;
                    curr.lineLen = pos + 1;
                    iniFSM(&curr, buff, linemax);
                }
                if(curr.nodeState != lcini_EMPTY && mylciniReadOutFunct != NULL ){   /* Dropping empty lines */
                        /*  mylciniReadOutFunct(line, pos+1, curr.section, curr.sectionLen, curr.param, curr.paramLen, curr.value, curr.valueLen, curr.comment, curr.commentLen, curr.errorMsg, curr.errorMsgLen);*/
                    (*mylciniReadOutFunct)(line, pos+1, curr.section, curr.sectionLen, curr.param, curr.paramLen, curr.value, curr.valueLen, curr.comment, curr.commentLen, curr.errorMsg, curr.errorMsgLen);
                }
    
                pos = 0;
                memset(buff, 0, linemax*sizeof(char));
            } else {
                buff[pos] = c;
                pos++;
            }
        }
    }

    if(fp){
        fclose(fp);
    }
    free(buff);
    return line;
}



