#include "util.h"
#include <string.h>
static char hex_c[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
char *random_access_code(char *buff,size_t buff_size){

    time_t now;
    time(&now);
    int rnd=rand();
    memset(buff,0,buff_size);
    size_t tlen=sizeof(time_t);
    size_t ilen=sizeof(int);
    unsigned char *r=malloc(tlen+ilen+1);
    memset(r,0,tlen+ilen+1);
    memcpy(r,&now,tlen);
    memcpy(r+tlen,&rnd,ilen);
    for(size_t i=0;i<((buff_size/2)>(tlen+ilen)?(tlen+ilen):(buff_size/2));i++){
        buff[2*i]=hex_c[(unsigned char)r[i]/16];
        buff[2*i+1]=hex_c[(unsigned char)r[i]%16];

    }


    return buff;
}
