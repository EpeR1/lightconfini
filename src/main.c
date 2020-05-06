#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h> 

#include <linux/limits.h> /* PATH_MAX*/
#include "ini_rw.h"
#define main_c


  
int main()
{
    char filename[] = "tests/test.ini", *buff1, *buff2, *buff3, *buff4, *buff5;
    lci_data *ini=NULL, *tmp=NULL;
    int len=0, elen=50, lens=0, lenp=0, lenv=0, lenc=0; 
    FILE *fp;

    fp = fopen(filename, "rb");
    len = getFileMaxLineLen(fp);
    fclose(fp);

    buff1 = malloc((len+3)*sizeof(char));
    buff2 = malloc((len+3)*sizeof(char));
    buff3 = malloc((len+3)*sizeof(char));
    buff4 = malloc((len+3)*sizeof(char));
    buff5 = malloc((elen+3)*sizeof(char));


    ini = iniReadOut(filename);


    printf("\n"); 

    lens=18;
    lenp=18;
    lenv=18;
    lenc=44;


    tmp = ini;
    while(tmp != NULL){

        snprintf(buff1, len+3, "'%s' %3ld",tmp->section, tmp->sectionLen);
        snprintf(buff2, len+3, "'%s' %3ld",tmp->param, tmp->paramLen);
        snprintf(buff3, len+3, "'%s' %3ld",tmp->value, tmp->valueLen);
        snprintf(buff4, len+3, "'%s' %3ld",tmp->comment, tmp->commentLen);
        snprintf(buff5, elen, "'%s' %3ld", tmp->errorMsg, tmp->errorMsgLen);


        printf("LN: %ld,\tLL: %ld,\tSE: %*s,%2ld P: %*s,%2ld V: %*s,%2ld C: %*s,%2ld ER: %*s \n", tmp->lineNum, tmp->lineLen, lens, buff1,tmp->sectionStartPos,
            lenp, buff2, tmp->paramStartPos, lenv, buff3, tmp->valueStartPos, lenc, buff4, tmp->commentStartPos, elen, buff5);

        tmp=tmp->next;
    }




    return 0;
}
