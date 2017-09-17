#include <sqlite3.h>
#include "system_constroller.h"
#include "logfile.h"
#include "sens_server.h"


static int sens_login_handle(struct sens_connection_t *conn,const struct sens_msg_head_t *head,const struct sens_msg_login_t *body){


    return LOGIN_OK;
}
static void sens_logout_handle(struct sens_connection_t *conn){

}
static int sens_connect_handle(struct sens_connection_t *conn){

    return CONNECT_ALLOW;
}

static void sens_disconnect_handle(struct sens_connection_t *conn){

}

static int sens_message_handle(struct sens_connection_t *conn,const struct sens_msg_head_t *head,char *body,size_t body_size){


    return RETURN_OK;
}


int sys_init(const struct sys_config_t* cfg){

    sens_config sens_cfg={0};

    sens_cfg.connection_size=cfg->sens_connection_size;
    strcpy(sens_cfg.addr,cfg->sens_addr);
    sens_cfg.loghnd=log_writer;
    sens_cfg.port=cfg->sens_port;
    sens_cfg.recv_timeout=cfg->sens_recv_timout;
    sens_cfg.send_timeout=cfg->sens_send_timeout;
    sens_cfg.auth_type=cfg->sens_auth_type;
    sens_cfg.connect_handler=&sens_connect_handle;
    sens_cfg.disconnect_handler=&sens_disconnect_handle;
    sens_cfg.login_handler=&sens_login_handle;
    sens_cfg.logout_handler=&sens_logout_handle;
    sens_cfg.message_handler=&sens_message_handle;

    int return_code=sens_init(&sens_cfg);
    if(return_code!=RETURN_OK){
        LOG_ERROR(log_writer,"sensor server init fail,code:[%d] \n",return_code);

        return -1;
    }

    return RETURN_OK;
}
int sys_start(){
    int return_code=sens_start();
    if(return_code!=RETURN_OK){
        LOG_ERROR(log_writer,"sensor server start fail,code:[%d] \n",return_code);
        return -1;
    }
}
int sys_close(){


}
int sys_destroy(){
    sens_destroy();
}
