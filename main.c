#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "confparser.h"
#include "logfile.h"
#include "sens_server.h"

#define DEFAULT_CONFIG_FILE "home-center.conf"



struct log_t *log_writer=NULL;
static int load_config(struct conf_t *config, struct sys_config_t *params);


static int load_config(struct conf_t *config, struct sys_config_t *params){

    const char *hvalue=NULL;

    hvalue = conf_get_value(config,SENS_ADDR_KEY);
    if(hvalue!=NULL)
        strcpy(params->sens_addr,hvalue);
    else
        strcpy(params->sens_addr,SENS_DEFAULT_ADDR);


    hvalue = conf_get_value(config,SENS_PORT_KEY);
    if(hvalue!=NULL)
        params->sens_port=atoi(hvalue);
    else
        params->sens_port=SENS_DEFAULT_PORT;

    hvalue = conf_get_value(config,SENS_CONNECTION_SIZE_KEY);
    if(hvalue!=NULL)
        params->sens_connection_size=atoi(hvalue);
    else
        params->sens_connection_size=SENS_DEFAULT_CONNECTION_SIZE;

    hvalue = conf_get_value(config,SENS_RECV_TIMEOUT_KEY);
    if(hvalue!=NULL)
        params->sens_recv_timout=atoi(hvalue);
    else
        params->sens_recv_timout=SENS_DEFAULT_RECV_TIMEOUT;

    hvalue = conf_get_value(config,SENS_SEND_TIMEOUT_KEY);
    if(hvalue!=NULL)
        params->sens_send_timeout=atoi(hvalue);
    else
        params->sens_send_timeout=SENS_DEFAULT_SEND_TIMEOUT;

    hvalue=conf_get_value(config,CTRL_ADDR_KEY);
    if(hvalue!=NULL)
        strcpy(params->ctrl_addr,hvalue);
    else
        strcpy(params->ctrl_addr,CTRL_DEFAULT_ADDR);

    hvalue=conf_get_value(config,CTRL_PORT_KEY);
    if(hvalue!=NULL)
        params->ctrl_port=atoi(hvalue);
    else
        params->ctrl_port = CTRL_DEFAULT_PORT;

    hvalue=conf_get_value(config,CTRL_CONNECTION_SIZE_KEY);
    if(hvalue!=NULL)
        params->ctrl_connection_size=atoi(hvalue);
    else
        params->ctrl_connection_size=CTRL_DEFAULT_CONNECTION_SIZE;

    hvalue =conf_get_value(config,SYSTEM_LOG_KEY);
    if(hvalue!=NULL)
        strcpy(params->sys_log_file,hvalue);
    else
        strcpy(params->sys_log_file,SYSTEM_DEFUALT_LOG);

    hvalue =conf_get_value(config,SYSTEM_SERVER_NAME_KEY);
    if(hvalue!=NULL)
        strcpy(params->sys_server_name,hvalue);
    else
        strcpy(params->sys_server_name,SYSTEM_DEFUALT_SERVER_NAME);

    hvalue =conf_get_value(config,SENS_AUTH_TYPE_KEY);
    if(hvalue!=NULL){
        params->sens_auth_type=atoi(hvalue);

    }
    else
        params->sens_auth_type=SENS_DEFAULT_AUTH_TYPE;

    hvalue =conf_get_value(config,SYSTEM_DB_FILE_KEY);
    if(hvalue!=NULL){
        strcpy(params->sys_db_file,hvalue);

    }
    else
        strcpy(params->sys_db_file,SYSTEM_DEFUALT_DB_FILE);


    return 0;
}



void main_exit (int retval) {

    log_destroy(log_writer);

    //sens_close();
    //sens_destroy();
    sys_destroy();



    #ifdef MSWIN32
        WSACleanup();
    #endif // MSWIN32
    exit (retval);
}

/* osetreni signalu */
void signal_handler (int signum) {

  main_exit (EXIT_SUCCESS);
}
int each_cb(void *item,size_t idx,void *data){
    int *d=(int *)item;
    printf("out :%d,idx:%d \n",*d,idx);
}
int main(int argc,char **args)
{

    struct s_list_t *list=list_create();

    int a1=1;
    int a2=2;
    int a3=3;
    int a4=4;
    list_add(list,&a1);
    list_add(list,&a2);
    list_add(list,&a3);
    list_add(list,&a4);
    list_add(list,&a1);
    list_add(list,&a2);
    list_add(list,&a3);
    list_add(list,&a4);



    //list_remove(list,&a4);
    //void *data=NULL;
    LIST_EACH(list,data,idx)
    {
    if(data!=NULL)
        printf("data: %d \n",*(int *)data);
    }
    list_destroy(list,NULL);


    for(size_t i=0;i<list->size;i++){

        printf("%d \n",*(int *)list_get(list,i));
    }

    printf("is contain a2 :%d \n",list_contain(list,&a2));



    //char config_filename
    struct conf_t *confhd=NULL;
    int c_code=0;
    if(argc>1){
        c_code = conf_parse(&confhd,args[1]);
    }
    else{
        c_code = conf_parse(&confhd,DEFAULT_CONFIG_FILE);
    }

    if(c_code!=0){
        printf("config file '%s' not found !\n",DEFAULT_CONFIG_FILE);
        exit (EXIT_FAILURE);
        return -1;
    }
    struct sys_config_t sys_cfg={0};
    //conf_each(each_conf,confhd,&sys_cfg);
    if(load_config(confhd,&sys_cfg)==0){
        #ifndef NDEBUG
        printf("load config file success! \n");
        #endif // NDEBUG
    }
    else{
        #ifndef NDEBUG
        printf("load config file fail! \n");
        #endif // NDEBUG
    }

    conf_destory(confhd);

    if(log_create(&log_writer,LOG_LEVEL_ALL,LOG_LEVEL_SAVE_DEFAULT,sys_cfg.sys_log_file)!=0){
        printf("logfile:[%s] load error! \n",sys_cfg.sys_log_file);
        exit(0);
        return -1;
    }
    /* registrace signal handleru */
    signal (SIGINT, signal_handler);
    signal (SIGTERM, signal_handler);
    #ifndef MSWIN32
    signal (SIGHUP, signal_handler);
    signal (SIGPIPE, SIG_IGN);
    #endif // MSWIN32
    //
    /*use win32 net */
    #ifdef MSWIN32
        WSADATA wsaData;
        int ret=WSAStartup(MAKEWORD(2,2), &wsaData);
        if(ret!=0){


            exit (EXIT_FAILURE);
        }
    #endif // OS_WINDOWS

    sys_init(&sys_cfg);
    sys_start();



    //log_destroy(log);
    LOG_INFO(log_writer,"home-center start success! \n");
    for(;;);

    main_exit (EXIT_SUCCESS);
    return 0;
}
