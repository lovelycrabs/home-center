#ifndef CONFPARSER_H_INCLUDED
#define CONFPARSER_H_INCLUDED

#define CONF_FILE_PARSE_OK 0
#define CONF_FILE_NOTEXISTS 1
#define CONF_FILE_PARSE_ERROR 2

#define EACH_CONTINUE 0
#define EACH_FOUND 1

#define CONF_GET_INT(conf,key) \
    atoi(conf_get_value(conf,key))

#define CONF_GET_DOUBLE(conf,key) \
    atof(conf_get_value(conf,key))






struct hash_t{
    char *key;
    char *value;
    struct hash_t *next;

};

typedef struct conf_t{
    struct hash_t *head;
    struct hash_t *end;
    size_t size;

} conf,*pconf;
typedef int (*each_func)(struct hash_t *,void *params);


int conf_parse(struct conf_t **conf,const char *filename);
void conf_each(each_func func,struct conf_t *conf,void  *params);
const char *conf_get_value(struct conf_t *conf,const char *key);
int conf_destory(struct conf_t *conf);

#endif // CONFPARSER_H_INCLUDED
