#ifndef SENS_SERVER_H_INCLUDED
#define SENS_SERVER_H_INCLUDED
#include "common.h"
#ifdef PTHREADS
#include <pthread.h>
#endif // PTHREADS

#define CONNECT_ALLOW 0
#define CONNECT_DENY  -1

#define LOGIN_OK 0
#define LOGIN_ID_NOTFOUND 1
#define LOGIN_PASS_FALSE 2
#define LOGIN_PERMISSION_DENIED 3

enum SENS_CLIENT_MSG{
    SENS_CLIENT_MSG_GET_LOGIN_PARAMS=0,
    SENS_CLIENT_MSG_LOGIN=1,
    SENS_CLIENT_MSG_LOGOUT =2,
    SENS_CLIENT_MSG_BEAT = 3,
    SENS_CLIENT_MSG_SENSOR_REG =4,
    SENS_CLIENT_MSG_SENSOR_DATA =5
};
enum SENS_S_MSG{
    SENS_S_MSG_LOGIN_PARAMS =0,
    SENS_S_MSG_LOGIN_RESP = 1,
    SENS_S_MSG_LOGOUT_RESP = 2,
    SENS_S_MSG_BEAT_RESP = 3,
    SENS_S_MSG_SENSOR_REG_RESP = 4,
    SENS_S_MSG_DATA =5,
    SENS_S_MSG_CLOSE =6,
    SENS_S_MSG_LOGIN_REQUIRE=7,

    SENS_S_MSG_OUT_CONN_SIZE = 2000,
    SENS_S_MSG_DATA_TYPE_UNSUPORT = 2001,
    SENS_S_MSG_DATA_RESOLVE_ERROR = 2002,
    SENS_S_MSG_INTERNAL_ERROR = 2003,
    SENS_S_MSG_ACCESS_DENIED = 2004

};
typedef struct sens_msg_head_t{
    short msg_type;
    union h_version_u version;
    char keepalive;
    char need_resp;
    short flag;
    //char device_id[ID_STR_LENGTH];

}PACKED sens_msg_head;


typedef struct sens_msg_accessinfo_t{

    char access_code[ID_STR_LENGTH];
    time_t sys_datetime;
    short auth_type;
}PACKED sens_msg_accessinfo;

typedef struct sens_msg_login_t{
    char deviceid[ID_STR_LENGTH];
    char pass_key[PASS_KEY_STR_LENGTH];
}PACKED sens_msg_login;
typedef struct sens_msg_login_resp_t{
    int login_flag;
    char access_code[ID_STR_LENGTH];
}PACKED sens_msg_login_resp;

typedef struct sens_config_t{
    unsigned short port;
    char addr[IP_ADDR_STR_LENGTH];
    size_t connection_size;
    struct log_t *loghnd;
    unsigned int recv_timeout;
    unsigned int send_timeout;
    short auth_type;
    int (*login_handler)(struct sens_connection_t *,const struct sens_msg_head_t *,const struct sens_msg_login_t *body);
    int (*connect_handler)(struct sens_connection_t *);
    void (*disconnect_handler)(struct sens_connection_t *);
    void (*logout_handler)(struct sens_connection_t *);
    int (*message_handler)(struct sens_connection_t *,const struct sens_msg_head_t *,char *body,size_t bodysize);
    //struct log_t *log_writer;

} sens_config;

typedef struct sens_connection_t{
    SOCKET client;
    struct sockaddr_in client_addr;
    char access_code[ID_STR_LENGTH];
    char logined;
    time_t last_active_time;

} sens_connection;


typedef struct sens_context_t{
    SOCKET sock;
    struct sockaddr_in addr;
    struct log_t *loghandle;
    struct sens_connection_t **connections;
    size_t max_connection_size;
    size_t cur_connection_count;
    int closed_flag;
    short auth_type;
    int (*login_handler)(struct sens_connection_t *,const struct sens_msg_head_t *,const struct sens_msg_login_t *);
    int (*connect_handler)(struct sens_connection_t *);
    void (*disconnect_handler)(struct sens_connection_t *);
    void (*logout_handler)(struct sens_connection_t *);
    int (*message_handler)(struct sens_connection_t *,const struct sens_msg_head_t *,char *,size_t );

    #ifdef PTHREADS
    pthread_mutex_t connection_mutex;
    pthread_t run_pid;
    #endif // PTHREADS
    //struct sens_config_t *config;

} sens_context,*psens_context;



#define SENS_MSG_HEAD_LEN sizeof(struct sens_msg_head_t)

int sens_init(const struct sens_config_t *conf);
int sens_start();
void *sens_running(void *flag);
int sens_close();
int sens_send_error(SOCKET client,short msg_type);
int sens_send(const struct sens_connection_t *conn,struct sens_msg_head_t *head,void *body,size_t bodysize);
int sens_send_login_params(struct sens_connection_t *conn);
int sens_msg_handle(struct sens_connection_t *conn,const struct sens_msg_head_t *head,char *body,size_t body_size);
struct sens_connection_t **sens_create_connection();
int sens_close_connection(struct sens_connection_t *conn);
int sens_destroy_connection(struct sens_connection_t *conn);

int sens_destroy();

#endif // SENS_SERVER_H_INCLUDED
