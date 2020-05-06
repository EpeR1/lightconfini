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
    int len=0, elen=50; 
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


    tmp = ini;
    while(tmp != NULL){

        snprintf(buff1, len+3, "'%s' %3ld",tmp->section, tmp->sectionLen);
        snprintf(buff2, len+3, "'%s' %3ld",tmp->param, tmp->paramLen);
        snprintf(buff3, len+3, "'%s' %3ld",tmp->value, tmp->valueLen);
        snprintf(buff4, len+3, "'%s' %3ld",tmp->comment, tmp->commentLen);
        snprintf(buff5, elen, "'%s' %3ld", tmp->errorMsg, tmp->errorMsgLen);


        printf("LN: %ld,\tLL: %ld,\tSE: %*s, P: %*s, V: %*s, C: %*s, ER: %*s \n", tmp->lineNum, tmp->lineLen,len+6, buff1, len+6, buff2, len+6, buff3, len+6, buff4, elen, buff5);

        tmp=tmp->next;
    }




    return 0;
}
