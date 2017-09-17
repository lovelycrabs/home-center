/* logfile.c
 * vystup logu
 */

#include "logfile.h"

#include <stdio.h>

#include <stdarg.h>
#include <time.h>
#include <assert.h>



#ifdef PTHREADS
pthread_mutex_t log_mutex;
#endif

static char out_tab[5][12]={"info","warning","error","system","unkown"};

static const char* get_logtype_str(unsigned short lev);

static const char* get_logtype_str(unsigned short lev){

    switch(lev){
        case LOG_LEVEL_INFO:
            return out_tab[0];
        case LOG_LEVEL_WARN:
            return out_tab[1];
        case LOG_LEVEL_ERR:
            return out_tab[2];
        case LOG_LEVEL_SYS:
            return out_tab[3];
        default:
            return out_tab[4];

    }

}


int log_create(struct log_t **log,unsigned short lev1,unsigned short lev2,const char *filename){
    struct log_t *l=(struct log_t *)calloc(sizeof(struct log_t),1);

    #ifdef PTHREADS
    assert(pthread_mutex_init(&l->fs_mutex,NULL)==0);
    #endif // PTHREADS
    l->filename=calloc(strlen(filename)+1,1);
    //int f=open(filename,O_RDONLY|O_CREAT);

    strcpy(l->filename,filename);
    l->fs = fopen (l->filename, "a");
    if(!l->fs){
        free(l->filename);
        free(l);
        return -1;
    }
    l->print_lev=lev1;
    l->save_lev=lev2;
    *log=l;
    return 0;

}
int log_destroy(struct log_t *log){
    if(log==NULL) return -1;
    free(log->filename);
    fclose(log->fs);
    free(log);

    return 0;
}
int log_printf (const struct log_t *log,unsigned short lev, const char *format, ...) {


    #ifdef PTHREADS
    pthread_mutex_lock (&log->fs_mutex);
    #endif


    time_t curtime=0; struct tm *loctime; char buf[65]={0};
    curtime = time (NULL);
    loctime = localtime (&curtime);

    strftime (buf, 64, "%Y-%m-%d %X", loctime);
    va_list vl;
    va_start (vl, format);
    if((log->save_lev&lev)==lev){
        fprintf (log->fs, "[%s %s]",get_logtype_str(lev),buf);
        vfprintf (log->fs, format, vl);
        fflush(log->fs);
    }
    if((log->print_lev&lev)==lev){
      fprintf (stderr, "[%s %s]:",get_logtype_str(lev), buf);
      vfprintf (stderr, format, vl);
    }
    va_end (vl);

    #ifdef PTHREADS
    pthread_mutex_unlock (&log->fs_mutex);
    #endif
    return 0;


}
