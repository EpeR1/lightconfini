/*INI fájl olvasás */

#include <stdio.h>
#include <string.h> /* strncpy + strerror*/
#include <stdlib.h> /* malloc(), atoi(), exit(EXIT_FALIURE)*/
#include <ctype.h>  /*, isspace(), tolower() */
#include <errno.h>       /*errno*/
#include <stdarg.h>
#include <stdint.h> /* int64_t*/

#define ini_read_c
#include "ini_rw.h"


enum ini_states state = Start;
struct lci_data fsmdata;


size_t strNullLen(const char *str){
    if(str == NULL){
        return 0;
    } else { 
        return strlen(str);
    }
}

char *strResize(char *ptr, size_t oldsize, size_t newsize){
    //return ptr;
    char *tmp;

    if(newsize == 0){   //pucol
        free(ptr);
        return NULL;

    } else if(newsize != oldsize){       //Változtat
        tmp = (char *) malloc(newsize*sizeof(char));
        memset(tmp, 0, newsize);

        if(tmp == NULL){    //Hiba esetén nem nyúl hozzá
            return ptr;
        } else if(ptr == NULL) {
            return tmp;
        } else if(newsize > oldsize) {  // FEL
            strncpy(tmp, ptr, oldsize); // old < new
            free(ptr); 
            return tmp;
        } else if(newsize < oldsize){   //LE
            strncpy(tmp, ptr, newsize); // new < old
            free(ptr);
            return tmp;
        } else {            //Ide sosem jutunk
            return ptr;
        }
    } else {
        return ptr;
    }
}

char unescape(char c){
    if(c == 'n'){            //Newline
        return '\n';
    } else if(c == 'a'){     //Bell
        return '\a';
    } else if(c == 'b'){     //Backspace
        return '\b';
    } else if(c == 'f'){     //Formfeed Page Break
        return '\f';
    } else if(c == 'r'){     //Carrige return
        return '\r';
    } else if(c == 't'){     //Horizontal tab
        return '\t';
    } else if(c == 'v'){     //Vertical tab
        return '\v';
    } else if(c == 'e'){
        return 0x1B;
    /*} else if(c < 0x20){
        return '~';    */
    } else {                // jó az eredeti
        return c;  
    }
}

char eescape(char c){
    if(c == '\n'){            //Newline
        return 'n';
    } else if(c == '\a'){     //Bell
        return 'a';
    } else if(c == '\b'){     //Backspace
        return 'b';
    } else if(c == '\f'){     //Formfeed Page Break
        return 'f';
    } else if(c == '\r'){     //Carrige return
        return 'r';
    } else if(c == '\t'){     //Horizontal tab
        return 't';
    } else if(c == '\v'){     //Vertical tab
        return 'v';
    } else if(c == '\\'){    //Backslash
        return '\\';
    } else if(c == '\''){    //Apostrophe
        return '\'';
    } else if(c == '\"'){    //Double quotation mark
        return '\"';
    /*} else if(c < 0x20){
        return '~';*/
    } else {                // jó az eredeti
        return c;  
    }
}


int64_t getFileMaxLineLen(FILE *tfd){

    int64_t  c=0;
    int64_t i=0, max=0;

    if (tfd != NULL){
        while( c != EOF){
            c = fgetc(tfd);
            if (c == '\n' || c == EOF) {    //Line End
                i++;    // An "\n" is a character 
                if(max < i){
                    max = i;
                }
                i = 0;      // Recount on every newline
            } else {
                i++;
            }
        }
        fseek(tfd, 0, SEEK_SET);    //vissza az elejére
        return max;
    } else {                // File open error
        return -1;
    }       
}

struct lci_data *iniFSM(struct lci_data *data, const char *in, int64_t len){

    int64_t i,j, vallen=len; 
    enum ini_states pstate=Start, state=Start;
    char cc;

    if(data == NULL){
        return NULL;
    } else {

        for(i=0, j=0; i<len; i++, j++){
            cc = in[i];

            switch (state) { 
                case Start: 
                    if(data->nodeState == MULTILINE){       //Elküldjük gyűjteni
                        state = DqmW;   // Vagy ValW ??
                        j = data->valueLen-2;   //(x-1) => \0; (x-2) => j--;
                        data->value = strResize(data->value, data->valueLen, data->valueLen+len+1);   // strResize(ptr, oldsize, newsize 
                        vallen = data->valueLen+len+1;
                        data->nodeState = CONTINUE;
                        i--;
                        pstate = Start;
                        break;
                    } else if(in[i] == '\n' || in[i] == '\r' || in[i] == '\0'){   //sorvége
                        state = Stop;                   // Sorvége
                        i--;
                    } else if(isspace(in[i])){           //ISSPACE, de nem sorvége
                        state = BgnSp;
                    } else if(in[i] == ';' || in[i] == '#' ){    //Komment jel az első
                        j = -1;
                        state = CommEndW;
                        data->comment = strResize(data->comment, data->commentLen, len);
                        memset(data->comment, 0, len);
                        data->commentStartPos = i;
                        data->sectionStartPos = -1;   //Hogy pucoljon
                    } else if(in[i] == '['){     //Szekció kezdődik
                        j = -1;
                        state = SectEndW;
                        data->section = strResize(data->section, data->sectionLen, len);
                        memset(data->section, 0, len);
                        data->sectionStartPos = i;  //zárójeleket is veszi!
                    } else if(isalnum(in[i]) ){  //Változó jön
                        j = -1;
                        state = EqW1;    
                        data->param = strResize(data->param, data->paramLen, len);
                        memset(data->param, 0, len);
                        data->paramStartPos = i;
                        i--;
                    } else {    // control karakterek az 1. pos-ban
                        state = Error; 
                        i--;
                    }
                    data->nodeState = EMPTY;
                    pstate = Start;
                    break;


                case BgnSp:       //Space() at Begining
                    if (in[i] == '\n' || in[i] == '\r' ){
                        state = Stop;
                        i--;
                    } else if (in[i] == ';' || in[i] == '#'){   //Komment jön
                        j = -1;
                        state = CommEndW;
                        data->comment = strResize(data->comment, data->commentLen, len);
                        memset(data->comment, 0, len);
                        data->commentStartPos = i;
                        data->sectionStartPos = -1;      //Hogy pucoljon
                    } else if(in[i] == '['){     //Szekció lesz
                        j = -1;
                        state = SectEndW;
                        data->section = strResize(data->section, data->sectionLen, len);
                        memset(data->section, 0, len);
                        data->sectionStartPos = i;
                    } else if (isalnum(in[i])){  //Változó lesz belőle
                        j = -1;
                        state = EqW1;
                        data->param = strResize(data->param, data->paramLen, len);
                        memset(data->param, 0, len);
                        data->paramStartPos = i;
                        i--;
                    } else if(isspace(in[i]) /*|| in[i] == '\0' */ ){
                        state = BgnSp;
                    } else {
                        state = Error;
                        i--;
                    }
                    pstate = BgnSp;
                    break;
 

                case CommEndW:      //Innentől comment, a sor végéig
                    if(in[i] == '\n' || in[i] == '\r'){
                        state = Stop;
                        data->nodeState = READY;
                        data->comment[j] = '\0';
                        data->commentLen = j+1;
                        i--;
                    } else {
                        data->comment[j] = in[i];
                    }
                    pstate = CommEndW;
                    break;        


                case SectEndW:          // Section-t gyűjti
                    if(in[i] == ']'){
                        state = SectEndD;
                        data->section[j] = '\0';
                        data->sectionLen = j+1;
                        if(j == 0){ //empty section
                            state = Error;
                        }
                    } else if(isalnum(in[i]) || in[i]=='/' || in[i]==' ' || in[i]=='_' || in[i]=='-' || in[i]=='.'){   
                        data->section[j] = in[i];
                    } else /*if((in[i] >= 0x00 && in[i] < 0x20) || in[i] == ';' || in[i] == '#' )*/ {       // túl korai sorvég, SP(), vagy komment 
                        state = Error;
                        data->section[j] = '\0';
                        data->sectionLen = j+1;
                        i--;
                    } /*else {
                        data->section[j] = in[i];
                    }*/
                    pstate = SectEndW;
                    break;


                case SectEndD:          //Section begyűjtve, utána: SP(), sorvég, vagy komment
                    if(in[i] == '\n' || in[i] == '\r'){
                        state = Stop;
                        data->nodeState = READY;
                        i--;
                    }else if(isspace(in[i])){
                        state = SectEndD;             //marad helyben
                    } else if (in[i] == ';' || in[i] == '#'){
                        j = -1;
                        state = CommEndW;
                        data->comment = strResize(data->comment, data->commentLen, len);
                        memset(data->comment, 0, len);
                        data->commentStartPos = i;
                        data->nodeState = CONTINUE;
                    } else {
                        state = Error;      //hibás sor
                        i--;
                    }
                    pstate = SectEndD;
                    break;


                case EqW1:              //Egyenlő előtti (változó)
                    if(in[i] == '='){    //végén egyenlő
                        state = ValPSP;
                        data->param[j] = '\0';
                        data->paramLen = j + 1;
                    } else if(isalnum(in[i]) || in[i]=='[' || in[i]==']' || in[i]=='_' || in[i]=='-' || in[i]=='.'){   //A paraméter neve vagy tömb támogatása (Mint PHP-ben)
                        data->param[j] = in[i];
                    } else if(isspace(in[i]) && in[i] != '\r' && in[i] != '\n'){   //Végén csak space lehet, egyenlő nélkül sorvég nem
                        state = EqW2;
                        data->param[j] = '\0';
                        data->paramLen = j + 1;
                    } else {
                        state = Error;
                        i--;
                    }
                    pstate = EqW1;
                    break;


                case EqW2:              //Egyenlő előtti (space)
                    if(in[i] == '='){    //Végre egyenlő
                        j = -1;
                        state = ValPSP;
                    } else if(isspace(in[i])){   //marad helyben
                        state = EqW2;
                    } else {
                        state = Error;
                        i--;
                    }
                    pstate = EqW2;
                    break;


                case ValPSP:    //EqD              //megvolt az egyenlő, érték előtti space
                    if(in[i] == '\n' || in[i] == '\r' /*|| in[i] == '\0'*/ ){   //ekkor üres változó
                        state = Stop;
                        data->nodeState = READY;
                        i--;
                    } else if(in[i] == ';' || in[i] == '#'){  //komment jön
                        j = -1;
                        state = CommEndW;
                        data->comment = strResize(data->comment, data->commentLen, len);
                        memset(data->comment, 0, len);
                        data->commentStartPos = i;
                        data->nodeState = CONTINUE;

                    } else if( pstate != Bslsh && in[i] == '\"' ){   //Macskaköröm jött
                        j = -1;
                        state = DqmW;
                        data->value = strResize(data->value, data->valueLen, len);
                        memset(data->value, 0, len);
                        data->valueStartPos = i;
                        data->nodeState = CONTINUE;
                    } else if(isspace(in[i])){                      //SPACE jött -> marad helyben
                        state = ValPSP;              
                    } else if(isalnum(in[i]) || in[i]=='-') {                     //Változó macskaköröm nélkül
                        j = -1;
                        state = ValW;
                        data->value = strResize(data->value, data->valueLen, len);
                        memset(data->value, 0, len);
                        data->valueStartPos = i;
                        data->nodeState = CONTINUE;
                        i--;
                    } else {
                        state = Error;
                        i--;
                    }
                    pstate = ValPSP;
                    break;


                case ValW:   //Érték kigyűjtése
                    if(in[i] == '\n' || in[i] == '\r'){   //vége -> új sor
                        state = Stop;
                        data->nodeState = READY;
                        data->value[j] = '\0';
                        data->valueLen = j + 1;
                        i--;
                    } else if(in[i] == ';' || in[i] == '#'){   //comment
                        data->nodeState = CONTINUE;
                        data->value[j] = '\0';
                        data->valueLen = j+1;
                        state = CommEndW;
                        j = -1;
                        data->comment = strResize(data->comment, data->commentLen, len);
                        memset(data->comment, 0, len);
                        data->commentStartPos = i;
                        data->nodeState = CONTINUE;
                    //} else if( in[i] == '\\' ){  //Backslash jött  
                    //    j--;            //A '\' nem számít bele!
                    //    state = Bslsh;
                    } else if(  isalnum(in[i]) || in[i]=='_' || in[i]=='-' || in[i]=='.' ){           //Normál betűk, space() csak idézőjelben jöhet!
                        data->value[j] = in[i];
                    } else if(isspace(in[i])){            //Aposztróf nélüli space -> sorvég
                        data->value[j] = '\0';
                        data->valueLen = j+1;
                        data->nodeState = READY;
                        state = ValFSP;
                        i--;
                    } else {
                        state = Error;
                        i--;
                    }
                    pstate = ValW;
                    break;


                case Bslsh: //Backslash -> escape-re lesz szükség 
                    if(pstate == ValW){
                        state = ValW;
                        data->value[j] = unescape(in[i]);
                    } else if(pstate == SectEndW){
                        state = SectEndW;
                        data->section[j] = unescape(in[i]);
                    }else if(pstate == DqmW){
                        state = DqmW;
                        data->value[j] = unescape(in[i]);
                        if(in[i] == '\n'){      //Workaround for UNIX line end
                            i--;
                        }
                        if( in[i] == '\r'){     //Workaround for WIN line endings
                            j--;
                        }
                    } else {
                        state = Error;
                        i--;
                    }
                    if(0x00 < in[i] && in[i] < 0x20){   //láthatatlan: pl \\n (escapelt sorvége)
                        //j--;
                    }
                    pstate = Bslsh;
                    break;


                case ValFSP:    //Érték utáni space
                    if( in[i] == '\n' || in[i] == '\r'){
                        state = Stop;
                        i--;
                    } else if( in[i] == ';' || in[i] == '#'){
                        j = -1;
                        state = CommEndW;
                        data->comment = strResize(data->comment, data->commentLen, len);
                        memset(data->comment, 0, len);
                        data->commentStartPos = i;
                        data->nodeState = CONTINUE;
                    } else if(isspace(in[i])){
                        state = ValFSP;         //marad helyben
                    } else {
                        state = Error;
                        i--;
                    }
                    pstate = ValFSP;
                    break;


                case DqmW:   //dupla idézőjelben az érték
                    if( in[i] == '\"'){        //újabb idézőjel -> string vége
                        data->value[j] = '\0';
                        data->valueLen = j+1;
                        data->nodeState = READY;
                        state = ValFSP;
                    } else if ( in[i] == '\\'){    //Backslash jött
                        j--;
                        state = Bslsh;
                    } else if( in[i] == '\n' && in[i-1] == '\\'){ ///UNIX style
                        data->value[j] = '\0';    // '\\n' => '\n\0'
                        data->valueLen = j+1;
                        data->nodeState = MULTILINE;
                        state = Stop;
                        i--;
                    } else if( in[i] == '\n' && in[i-1] == '\r' && in[i-2] == '\\'){ //WINDOWS style
                        data->value[j]   = '\n';
                        data->value[j+1] = '\0';
                        data->valueLen = j+2;
                        data->nodeState = MULTILINE;
                        state = Stop;
                        i--;
                    } else if( pstate != Bslsh && ( in[i] == '\r' || in[i] == '\n' || in[i] == '\0')){       //túl korai sorvége
                        state = Error;
                        i--;
                    } else {
                        data->value[j] = in[i];
                    }
                    pstate = DqmW;
                    break; 


                case Error:
                    data->errorMsg = strResize(data->errorMsg, data->errorMsgLen, 256);
                    memset(data->errorMsg, 0, 256) ; 

                    if(pstate == SectEndW || pstate == SectEndD){
                        data->errorMsgLen = snprintf(data->errorMsg, 256, "Illegal character or EMPTY SECTION! (line: %ld, pos: %ld)", data->lineNum, i+1);
                    } else if(pstate == EqW1 || pstate == EqW2){
                        data->errorMsgLen = snprintf(data->errorMsg, 256, "Illegal character at PARAMETER! (line: %ld, pos: %ld)", data->lineNum, i+1);
                    } else if(pstate == ValPSP || pstate == ValW || pstate == ValFSP ){
                        data->errorMsgLen = snprintf(data->errorMsg, 256, "Illegal character at VALUE! (line: %ld, pos: %ld)", data->lineNum, i+1);
                    } else if(pstate == DqmW){
                        data->errorMsgLen = snprintf(data->errorMsg, 256, "Double quotation mark needed! (line: %ld, pos: %ld)", data->lineNum, i+1);
                    } else {    //pstate == Stop
                        data->errorMsgLen = snprintf(data->errorMsg, 256, "Illegal character! (line: %ld, pos: %ld)\0", data->lineNum, i+1)+1;
                    }

                    i--;
                    state = Stop;    
                    data->nodeState = ERROR;
                    pstate = Error;
                    break;

                case Stop:
                    if(in[i] == '\n' || in[i] == '\r' || in[i] == '\0' || pstate == Error){   //Sorvége, maradunk
                        state = Stop;
                        if(strNullLen(data->section) == 0){data->sectionLen = 0;}
                        if(strNullLen(data->param) == 0){data->paramLen = 0;}
                        if(strNullLen(data->value) == 0){data->valueLen = 0;}
                        if(strNullLen(data->comment) == 0){data->commentLen = 0;}
                        if(strNullLen(data->errorMsg) == 0){data->errorMsgLen = 0;}
                        data->section = strResize(data->section, len, data->sectionLen);
                        data->param = strResize(data->param, len, data->paramLen);
                        data->value = strResize(data->value, vallen, data->valueLen);
                        data->comment = strResize(data->comment, len, data->commentLen);
                        data->errorMsg = strResize(data->errorMsg, 256, data->errorMsgLen);
                        if(data->sectionStartPos < 0 && data->param==NULL && data->value==NULL && data->comment==NULL && data->errorMsg==NULL){
                            data->nodeState = EMPTY;
                        }
                        return data;
                    } else {                    //minden más-> hiba
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




lci_data *createNode( lci_data *head, int64_t lineLen ){
    lci_data *curr;

    curr = (lci_data *) malloc(1*sizeof(lci_data));
    curr->nodeState = EMPTY; 
    curr->lineNum = 0;
    curr->lineLen = lineLen;
    curr->section = (char *) malloc(lineLen*sizeof(char));
    curr->param = (char *) malloc(lineLen*sizeof(char));
    curr->value = (char *) malloc(lineLen*sizeof(char));
    curr->comment = (char *) malloc(lineLen*sizeof(char));
    curr->errorMsg = (char *) malloc(lineLen*sizeof(char));

    memset(curr->section, 0, lineLen);
    curr->sectionLen = lineLen;
    curr->sectionStartPos = -1;
    memset(curr->param, 0, lineLen);
    curr->paramLen = lineLen;
    curr->paramStartPos = -1;
    memset(curr->value, 0, lineLen);
    curr->valueLen = lineLen;
    curr->valueStartPos = -1;
    memset(curr->comment, 0, lineLen);  
    curr->commentLen = lineLen;
    curr->commentStartPos = -1;
    memset(curr->errorMsg, 0, lineLen);
    curr->errorMsgLen=lineLen;
    curr->next = NULL;

    if(head != NULL){
        head->next = curr;
    }
    return curr;
}       

lci_data *destroyNodes( lci_data *head){
    lci_data *tmp, *node=head;
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

struct lci_data *iniReadOut(const char *filename){

    int  c=0; 
    FILE *fp=NULL;
    int64_t linemax, line=0, pos=0;
    char *buff, cc;
    struct lci_data *prev=NULL, *curr=NULL, *list = NULL;

    fp = fopen(filename, "rb"); 
    if(fp == NULL){
        list = createNode(NULL, 120);
        list->errorMsgLen = snprintf(list->errorMsg, 120, "File opening error. Errno: %d (%s)", errno, strerror(errno) );
        list->nodeState = ERROR;
        //return list;
    } else {
        linemax = getFileMaxLineLen(fp);
        //list = createNode(NULL, linemax);
        buff = (char *) malloc(linemax*sizeof(char));
        memset(buff, 0, linemax);


        while( c != EOF){
            c = fgetc(fp);
            cc = c;

            if( c == '\n' || c == EOF){ //Ha sorvége
                line++;
                if(c == EOF){
                    buff[pos] = '\n';
                } else {
                    buff[pos] = cc;
                }

                if(curr == NULL || curr->nodeState != MULTILINE ){
                    curr = createNode(NULL, linemax);
                }
                if(list == NULL){
                    list = curr;
                }
                if(prev != NULL && prev != curr && prev->section != NULL && curr != NULL && curr->section != NULL){
                    strncpy(curr->section, prev->section, curr->sectionLen); 
                    curr->sectionLen = prev->sectionLen;
                    //curr->section = strResize(curr->section, linemax, curr->sectionLen);

                }
                if(curr != NULL){
                    curr->lineNum = line;
                    curr->lineLen = pos + 1;
                    curr = iniFSM(curr, buff, linemax);
                }
                if(curr->nodeState == EMPTY){   //üres sorokat eldobjuk
                    if(prev == curr || list == curr){   //Ha csak az az egy van a listában
                        list = NULL;
                        prev = NULL;
                    }
                    curr = destroyNodes(curr);
                //} else if(curr->nodeState = ERROR){
                    //destroyNodes(list);
                //} else if (curr->nodeState == MULTILINE){
                    //multiline
                } else {
                       //????????
                }


                if(prev == NULL){               //Ha még üres
                    prev = curr;
                } else {                        //Az előzőhöz hozzávesszük a mostanit
                    prev->next = curr;
                    if(prev->next != NULL && curr->nodeState != MULTILINE){  //Csak, ha lett új felvéve
                        prev = prev->next;
                    }
                }
                
                if(curr != NULL && curr->nodeState == ERROR){
                    //return list;
                }
                pos = 0;
                memset(buff, 0, linemax);
            } else {
                buff[pos] = c;
                pos++;
            }
        }
        free(buff);
    }

    if(fp != NULL){
        fclose(fp);
    }
    return list;
}




