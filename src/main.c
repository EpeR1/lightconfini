#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>   

#include "ini_rw.h"
#define main_c  

 
  
int main(int argc, char* argv[]){


    char filename[4096] = "tests/test.ini", *buff1, *buff2, *buff3, *buff4, *buff5;
    lci_data *ini=NULL, *tmp=NULL;
    int len=0, elen=54, lens=0, lenp=0, lenv=0, lenc=0; 
    FILE *fp;

    if(argc > 1){
        memset(filename, 0, 4096);
        snprintf(filename, 4096, "%s", argv[1]);
    }


    fp = fopen(filename, "rb");
    len = getFileMaxLineLen(fp)+0;
    if(fp != NULL) { fclose(fp);}

    len = 24000;    //Debug
    lens=16;
    lenp=16;
    lenv=16;
    lenc=44;


    buff1 = calloc((len+3), sizeof(char));
    buff2 = calloc((len+3), sizeof(char));
    buff3 = calloc((len+3), sizeof(char));
    buff4 = calloc((len+3), sizeof(char));
    buff5 = calloc((elen+3),sizeof(char));


    printf("\nLineMax: %d\n\n",len); 

    ini = iniReadOut(filename);


    tmp = ini;
    while(tmp != NULL){

        snprintf(buff1, len+3, "'%s' %3ld",tmp->section, tmp->sectionLen);
        snprintf(buff2, len+3, "'%s' %3ld",tmp->param, tmp->paramLen);
        snprintf(buff3, len+3, "'%s' %3ld",tmp->value, tmp->valueLen);
        snprintf(buff4, len+3, "'%s' %3ld",tmp->comment, tmp->commentLen);
        snprintf(buff5, elen+3,  "'%s' %3ld",tmp->errorMsg, tmp->errorMsgLen);


        printf("LN: %ld,\tLL: %ld,\tSE: %*s,%2ld P: %*s,%2ld V: %*s,%2ld C: %*s,%2ld ER: %*s \n", tmp->lineNum, tmp->lineLen, lens, buff1,tmp->sectionStartPos,
            lenp, buff2, tmp->paramStartPos, lenv, buff3, tmp->valueStartPos, lenc, buff4, tmp->commentStartPos, elen, buff5);

        tmp=tmp->next;
    }

    free(buff1);
    free(buff2);
    free(buff3);
    free(buff4);
    free(buff5);     

    destroyNodes( ini);


    return 0;
}
