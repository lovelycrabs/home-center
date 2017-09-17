#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED
#include "global.h"

#ifdef MSWIN32

#include <winsock2.h>

#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int SOCKET;

#endif // MSWIN32

#ifdef MSWIN32
#define CLOSE_SOCK(so) closesocket(so)
#else
#define CLOSE_SOCK(so) close(so)
#endif // MSWIN32

#define SERVER_VERSION 0x01000200 //1.0.2.0

#define RETURN_OK 0

#define SENS_PORT_KEY "sens_port"
#define SENS_DEFAULT_PORT 5612

#define SENS_CONNECTION_SIZE_KEY "sens_connection_size"
#define SENS_DEFAULT_CONNECTION_SIZE 32


#define CTRL_PORT_KEY "ctrl_port"
#define CTRL_DEFAULT_PORT 5610

#define CTRL_CONNECTION_SIZE_KEY "ctrl_connection_size"
#define CTRL_DEFAULT_CONNECTION_SIZE 10

#define SYSTEM_LOG_KEY "system_log_file"

#ifdef MSWIN32
#define SYSTEM_DEFUALT_LOG "log/home-center.txt"
#else
#define SYSTEM_DEFUALT_LOG "/var/log/home-center/home-center.log"
#endif // MSWIN32

#define SENS_ADDR_KEY "sens_addr"
#define SENS_DEFAULT_ADDR "0.0.0.0"

#define SENS_RECV_TIMEOUT_KEY "sens_recv_timeout"
#define SENS_DEFAULT_RECV_TIMEOUT 6000

#define SENS_SEND_TIMEOUT_KEY "sens_send_timeout"
#define SENS_DEFAULT_SEND_TIMEOUT 3000

#define SENS_AUTH_TYPE_KEY "sens_auth_type"
#define SENS_DEFAULT_AUTH_TYPE 1

#define CTRL_ADDR_KEY "ctrl_addr"
#define CTRL_DEFAULT_ADDR "0.0.0.0"

#define SYSTEM_SERVER_NAME_KEY "sys_server_name"
#define SYSTEM_DEFUALT_SERVER_NAME "smart home center"

#define SYSTEM_DB_FILE_KEY "db_file"
#define SYSTEM_DEFUALT_DB_FILE "home-center.db"


#define IP_ADDR_STR_LENGTH 32
#define FILE_NAME_STR_LENGTH 128
#define NAME_STR_LENGTH 32
#define ID_STR_LENGTH 32
#define PASS_KEY_STR_LENGTH 32

#define CONNECT_KEEPALIVED 1
#define CONNECT_CLOSED 0


#define SELECT_TIMEOUT 100000

#define MAX_BUFFER_SIZE 8196


enum AUTH_TYPE{
    AUTH_TYPE_SHA1=1,
    AUTH_TYPE_SHA256=2,
    AUTH_TYPE_MD5=3
};

typedef union h_version_u{
    unsigned ver;
    struct{
        char no3;
        char no2;
        char no1;
        char no0;
    } v_no;

} h_version;

typedef struct sys_config_t{
    unsigned short sens_port;
    char sens_addr[IP_ADDR_STR_LENGTH];
    unsigned short ctrl_port;
    char ctrl_addr[IP_ADDR_STR_LENGTH];
    size_t sens_connection_size;
    size_t ctrl_connection_size;
    char sys_log_file[FILE_NAME_STR_LENGTH];
    char sys_server_name[NAME_STR_LENGTH];
    char mac_filter;
    unsigned int sens_recv_timout;
    unsigned int sens_send_timeout;
    short sens_auth_type;
    char sys_db_file[FILE_NAME_STR_LENGTH];

} sys_config;

struct list_node_t{
    void *value;
    struct list_node *next;

};

struct s_list_t{
    struct list_node_t *head;
    struct list_node_t *last;
    size_t size;
};


struct s_list_t *list_create();
void list_destroy(struct s_list_t *list,void (*each)(void *item,size_t idx));
size_t list_add(struct s_list_t *list,void *item);
int list_remove(struct s_list_t *list,void *item);
void *list_remove_at(struct s_list_t *list,size_t index);
int list_contain(struct s_list_t *list,void *item);
void list_each(struct s_list_t *list,int (*each)(void *,size_t ,void *),void *data);
void *list_get(struct s_list_t *list,size_t index);
size_t list_size(struct s_list_t *list);


#define LIST_EACH(list,cur,idx) \
    size_t idx=0;\
    struct list_node_t *s_cur=list->head;\
    void *cur=s_cur->value;\
    for(;s_cur;s_cur=s_cur->next,cur=(s_cur==NULL?NULL:s_cur->value),idx++)




#endif // COMMON_H_INCLUDED
