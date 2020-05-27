#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>   
/*#define TESTLIBD*/
/*#define LIBINSTALLED*/
#ifdef TESTLIBD
#include <dlfcn.h>
#endif
#ifdef LIBINSTALLED
#include <lightconfini.h>
#else
#include "lightconfini.h"
#endif
#include "main.h"
#define main_c  

#ifdef LIBINSTALLED 
const char* linkpath = "liblightconfini.so.1";
#else
const char* linkpath = "./libdir/liblightconfini.so.1";
#endif


/* int lens=46, lenp=36, lenv=56, lenc=74, elen=65; */
int lens=16, lenp=16, lenv=16, lenc=24, elen=55;
lcinimyReadFunc mylciniReadOutFunct=myfunct;
/*lcinimyReadFunc  *mylciniReadOutFunct=NULL;*/
char *search_sec = "bom_section";
char *search_par = "mmm";



void myfunct(int line, int linelen, char *section, int sectionlen, char *param, int paramlen, char *value, int valuelen, char *comment, int commentlen, char *error, int errorlen ){
    
    printf("LN: %d,\tLL: %d, \tSC: %*s,%2d P: %*s,%2d V: %*s,%2d C: %*s,%2d ER: %*s,%2d\n", line, linelen, lens, section,sectionlen,
     lenp, param, paramlen, lenv, value, valuelen, lenc, comment, commentlen, elen, error, errorlen);
}


int main(int argc, char* argv[]){
 
    int len,r;
    char filename[4096] = "tests/test.ini", *buff1, *buff2, *buff3, *buff4, *buff5, *buff6;
    lcini_data *ini=NULL, *tmp=NULL;
    FILE *fp;
    lcini_shortret *sret=NULL, *sret2=NULL;
#ifdef TESTLIBD
    void *hl;
    size_t              (*lciniFileMaxLineLen)(FILE *tfd);
    struct lcini_data  *(*lciniReadOut)(const char *filename);
    int                 (*lciniReadOutOwn)(const char *filename);
    lcini_data         *(*lciniGet)(lcini_data *head, const char *section, const char *parameter); 
    int                 (*lciniGetStr)(lcini_data *head, const char *section, const char *parameter, char *dst, int dstlen);
    lcini_shortret     *(*lciniGetShort)(lcini_data *head, const char *section, const char *parameter, lcini_shortret *ret);
    lcini_shortret     *(*lciniGetFromFileShort)(const char *filename, const char *section, const char *parameter, lcini_shortret *ret);
    int                 (*lciniGetFromFileStr)(const char *filename, const char *section, const char *parameter, char *dst, int dstlen);
    lcini_data         *(*lciniDestroyNodes)( lcini_data *head); 
    /*lcini_data         *(*lciniCreateNode)( lcini_data *head, int lineLen ); */
    /*lcini_shortret     *(*lciniMKShortRet)(int bufflen); */
    void                (*lciniDestroyShortRet)(lcini_shortret *dt);
    char               *(*lciniGetVersionStr)(void);
#endif
    if(argc > 1){
        memset(filename, 0, 4096);
        /* snprintf(filename, 4096, "%s", argv[1]); */
        sprintf(filename, "%s", argv[1]);
    }
    if(argc > 2){
        search_sec = argv[2];
    }
    if(argc > 3){
        search_par = argv[3];
    }

#ifdef TESTLIBD
    hl = dlopen(linkpath, RTLD_LAZY);
    if(!hl){
        printf("\nDynamic Library load error - %s \nexit\n",dlerror());
        exit(1);
    } else {
        printf("\nDynamic Library loaded: %s", linkpath);
    }
    lciniFileMaxLineLen = dlsym(hl, "lciniFileMaxLineLen");
    if((buff1 = dlerror()) != NULL){
        printf("\nsym err - %s\n", buff1);
    }
    lciniReadOut = dlsym(hl, "lciniReadOut");
    if((buff1 = dlerror()) != NULL){
        printf("\nsym err - %s\n", buff1);
    }
    lciniReadOutOwn = dlsym(hl, "lciniReadOutOwn");
    if((buff1 = dlerror()) != NULL){
        printf("\nsym err - %s\n", buff1);
    }
    lciniGet = dlsym(hl,"lciniGet");
    if((buff1 = dlerror()) != NULL){
        printf("\nsym err - %s\n", buff1);
    }
    lciniGetStr = dlsym(hl,"lciniGetStr");
    if((buff1 = dlerror()) != NULL){
        printf("\nsym err - %s\n", buff1);
    }
    lciniGetShort = dlsym(hl,"lciniGetShort");
    if((buff1 = dlerror()) != NULL){
        printf("\nsym err - %s\n", buff1);
    }
    lciniGetFromFileShort = dlsym(hl, "lciniGetFromFileShort");
    if((buff1 = dlerror()) != NULL){
        printf("\nsym err - %s\n", buff1);
    }
    lciniGetFromFileStr = dlsym(hl,"lciniGetFromFileStr");
    if((buff1 = dlerror()) != NULL){
        printf("\nsym err - %s\n", buff1);
    }
    lciniDestroyNodes = dlsym(hl,"lciniDestroyNodes");
    if((buff1 = dlerror()) != NULL){
        printf("\nsym err - %s\n", buff1);
    }
    /*lciniCreateNode = dlsym(hl,"lciniCreateNode");
    if((buff1 = dlerror()) != NULL){
        printf("\nsym err - %s\n", buff1);
    }*/
    /*lciniMKShortRet = dlsym(hl,"lciniMKShortRet");
    if((buff1 = dlerror()) != NULL){
        printf("\nsym err - %s\n", buff1);
    }*/
    lciniDestroyShortRet = dlsym(hl,"lciniDestroyShortRet");
    if((buff1 = dlerror()) != NULL){
        printf("\nsym err - %s\n", buff1);
    }
    lciniGetVersionStr = dlsym(hl, "lciniGetVersionStr");
    if((buff1 = dlerror()) != NULL){
        printf("\nsym err - %s\n", buff1);
    }

#endif


    fp = fopen(filename, "rb");
    len = lciniFileMaxLineLen(fp)+1;
    if(fp != NULL) { fclose(fp);}

    /* len = 24000; */    /*Debug*/

    buff1 = calloc((len+100), sizeof(char));
    buff2 = calloc((len+100), sizeof(char));
    buff3 = calloc((len+100), sizeof(char));
    buff4 = calloc((len+100), sizeof(char));
    buff5 = calloc((elen+100),sizeof(char));
    buff6 = calloc(100, sizeof(char));


    printf("\nLineMax: %d\n\n",len); 
    printf("FUNC: lciniReadOut() \n\n");

    ini = lciniReadOut(filename);
    tmp = ini;
    while(tmp != NULL){ 
    /*    snprintf(buff1, len+3, "'%s' %3ld",tmp->section, tmp->sectionLen);
        snprintf(buff2, len+3, "'%s' %3ld",tmp->param, tmp->paramLen);
        snprintf(buff3, len+3, "'%s' %3ld",tmp->value, tmp->valueLen);
        snprintf(buff4, len+3, "'%s' %3ld",tmp->comment, tmp->commentLen);
        snprintf(buff5, elen+3,  "'%s' %3ld",tmp->errorMsg, tmp->errorMsgLen); */

        sprintf(buff1, "'%s' %3d",tmp->section, tmp->sectionLen);
        sprintf(buff2, "'%s' %3d",tmp->param, tmp->paramLen);
        sprintf(buff3, "'%s' %3d",tmp->value, tmp->valueLen);
        sprintf(buff4, "'%s' %3d",tmp->comment, tmp->commentLen);
        sprintf(buff5, "'%s' %3d",tmp->errorMsg, tmp->errorMsgLen);


        printf("LN: %d,\tLL: %d, \tSC: %*s,%2d P: %*s,%2d V: %*s,%2d C: %*s,%2d ER: %*s \n", tmp->lineNum, tmp->lineLen, lens, buff1,tmp->sectionStartPos,    lenp, buff2, tmp->paramStartPos, lenv, buff3, tmp->valueStartPos, lenc, buff4, tmp->commentStartPos, elen, buff5); 
        /* myfunct(tmp->lineNum, tmp->lineLen, (char*)tmp->section, tmp->sectionLen, (char*)tmp->param, tmp->paramLen, (char*)tmp->value, tmp->valueLen, (char*)tmp->comment, tmp->commentLen, (char*)tmp->errorMsg, tmp->errorMsgLen); */

        tmp=tmp->next;
    }
    
  

    printf("\n\nFUNC: lciniReadOutOwn():\n\n");
    lciniReadOutOwn(filename); 




    printf("\n\nFUNC: lciniGet():\n\n");
    tmp = lciniGet(ini, search_sec, search_par);
    printf("X->%p\n", (void *)tmp);
    if(tmp){
        printf("x->S: '%s', x->P: '%s', x->V: '%s', x->E: '%s'\n",tmp->section, tmp->param, tmp->value, tmp->errorMsg);
    }


    printf("\n\nFUNC: lciniGetStr():\n\n");
    r=lciniGetStr(ini, search_sec, search_par, buff6, 100);
    printf("r: %d, R: '%s' \n",r,buff6);



    printf("\n\nFUNC: lciniGetShort():\n\n");
    sret = lciniGetShort(ini, search_sec, search_par, NULL);
    printf("SR: '%s', %d, t: ",sret->ret,sret->retlen);
    if(sret->retType == lcini_shortretEMPTY){
        printf("sret_empty\n");
    }else if(sret->retType == lcini_shortretERROR){
        printf("sret_err\n");
    } else {
        printf("sret_ok\n");
    }


    printf("\n\nFUNC: lciniGetFromFileShort():\n\n");
    sret2 = lciniGetFromFileShort(filename, search_sec, search_par, sret2);
    printf("---\n");
    if(sret2){
        printf("SR: '%s', %d, t: ",sret2->ret,sret2->retlen);
        if(sret2->retType == lcini_shortretEMPTY){
            printf("sret_empty\n");
        }else if(sret2->retType == lcini_shortretERROR){
            printf("sret_err\n");
        } else {
            printf("sret_ok\n");
        }
    }

    printf("\n\nFUNC: lciniGetFromFileStr():\n\n");
    r=lciniGetFromFileStr(filename, search_sec, search_par, buff6, 100); 
    printf("r: %d, R: '%s' \n",r,buff6);


    lciniDestroyShortRet(sret2);
    lciniDestroyShortRet(sret);
    lciniDestroyNodes(ini);
    free(buff1);
    free(buff2);
    free(buff3);
    free(buff4);
    free(buff5); 
    free(buff6);
    printf("\n\nLightConfINI Version: %s \n", lciniGetVersionStr());
    printf("\n -- end --\n");
#ifdef TESTLIBD
    dlclose(hl);
#endif
    return 0;
}
