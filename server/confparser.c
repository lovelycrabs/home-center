#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
//#include <assert.h>
#include "confparser.h"
#define TRIM_LEFT 0x01
#define TRIM_RIGHT 0x02
#define TRIM_ALL 0x03


int set_zero(char *line);
int trim(char *out,const char *in,unsigned char flag);
struct hash_t *create_hash(const char *line);


void conf_add(struct conf_t *conf,struct hash_t *hast);


int set_zero(char *line){
    size_t len=strlen(line);
    size_t ret=len;
    for(size_t i=0;i<len;i++){

        if(line[i]=='#'){
            for(size_t k=i;k<len;k++){
                line[k]=0;
                ret--;
            }

        }
        else if(line[i]=='\r'||line[i]=='\n'){
            line[i]=0;
            ret--;
        }
    }
    return ret;

}
int trim(char *out,const char *in,unsigned char flag){
    size_t inlen=strlen(in);
    char *newline=malloc(inlen+1);
    char *line=newline;
    strcpy(line,in);
    if((flag&0x02)==0x02){
        while(isspace(line[inlen-1])){
            line[inlen-1]=0;
            inlen--;
        }
    }

    if((flag&0x01)==0x01){
        while(isspace(*line++)){
            inlen--;
        }
        line--;
    }

    strcpy(out,line);
    free(newline);
    return inlen;

}
struct hash_t *create_hash(const char *line){
    if(line==NULL) return NULL;
    size_t linesize=strlen(line);
    if(linesize==0) return NULL;

    struct hash_t *kv=(struct hash_t *)calloc(sizeof(struct hash_t),1);

    size_t split_flag_idx=linesize;
    for(size_t i=0;i<linesize;i++){
        if(line[i]=='='){
            split_flag_idx=i;

            break;
        }

    }
    kv->key=(char *)calloc(split_flag_idx+1,1);

    char *key=(char *)calloc(split_flag_idx+1,1);
    strncpy(key,line,split_flag_idx);
    trim(kv->key,key,TRIM_RIGHT);
    free(key);

    if(linesize==split_flag_idx){
        kv->value=NULL;
    }else{
        char *value=line+split_flag_idx+1;
        kv->value=(char *)calloc(linesize-split_flag_idx,1);
        trim(kv->value,value,TRIM_LEFT);
    }


    return kv;



}
void conf_add(struct conf_t *conf,struct hash_t *hash){
    if(hash==NULL) return;
    if(conf->head==NULL){
        conf->head=hash;
        conf->end=hash;
        conf->size++;
        return;
    }
    if(conf->size==1){
        conf->head->next=hash;
        conf->end=hash;

    }
    else{
        conf->end->next=hash;
        conf->end=hash;
    }
    conf->size++;

}
int conf_parse(struct conf_t **out,const char *filename){

   struct conf_t *conf=(struct conf_t *)calloc(sizeof(struct conf_t),1);
   FILE *fp=NULL;
   //打开配置文件，返回FILE指针

    if((fp=fopen(filename,"rb")) == NULL){
        #ifndef NDEBUG
        printf("打开%s出错!\n",filename);
        #endif // NDEBUG

        return 1;
    }

    ssize_t size=0;//存放 当前行 的长度（包括“\n”）

    size_t len = 0;//文件总长度

    char * line = NULL;//存放 当前行 的字符串


    while((size=getline(&line,&len,fp))!=-1){
        if(size>1){
            size_t proclen=set_zero(line);
            if(proclen>0){
                char *trimed=calloc(size+1,1);
                trim(trimed,line,TRIM_ALL);
                if(strlen(trimed)>0){
                    struct hash_t *kv=create_hash(trimed);
                    conf_add(conf,kv);
                }

                free(trimed);
            }
        }


        len=0;
        size=0;

        //printf("k:%s,v:%s \n",kv->key,kv->value);
        free(line);
        line=NULL;
    }

    fclose(fp);
    *out=conf;
    return 0;


}
void  conf_each(each_func func,struct conf_t *conf,void  *params){

    struct hash_t *kv=conf->head;
    while(kv!=NULL){
        if(func(kv,params)!=EACH_CONTINUE) break;
        kv=kv->next;
    }
}

const char *conf_get_value(struct conf_t* conf,const char *key){


    struct hash_t *kv=conf->head;
    while(kv!=NULL){
        if(kv->key==NULL) continue;

        if(strcmp(key,kv->key)==0||kv->key==key)
            return kv->value;

        kv=kv->next;
    }
    return NULL;
}

int conf_destory(struct conf_t *conf){
    //printf("free start !\n");
    struct hash_t *kv=conf->head;
    while(kv!=NULL){
        struct hash_t *cur=kv->next;
        //printf("key:%s,value:%s \n",kv->key,kv->value);
        if(kv->key) free(kv->key);
        if(kv->value) free(kv->value);
        free(kv);
        //kv=NULL;
        kv=cur;
    }
    free(conf);
    //printf("free end !\n");
    return 0;
}
