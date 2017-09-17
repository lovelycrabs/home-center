#include "sens_server.h"


#include <assert.h>
#include "logfile.h"
#include "util.h"

static struct sens_context_t *context;
static SOCKET get_max_sock();
static int  socket_init(const struct sens_config_t *conf);
static int  remove_connection(struct sens_connection_t **conn,fd_set *set,SOCKET *maxfd);

static SOCKET get_max_sock(){
    SOCKET maxfd=context->sock;


    for(size_t i=0;i<context->max_connection_size;i++){
        if(context->connections[i]!=NULL){
            if(context->connections[i]->client>maxfd){

                maxfd=context->connections[i]->client;

            }
           // FD_SET(context->connections[i]->client,&context->socket_set);
        }
    }
    return maxfd;

}

static int  remove_connection(struct sens_connection_t **conn,fd_set *set,SOCKET *maxfd){

    printf("client %s:%d disconnected \n",inet_ntoa((*conn)->client_addr.sin_addr),ntohs((*conn)->client_addr.sin_port));

    context->disconnect_handler(*conn);
    FD_CLR((*conn)->client,set);
    if((*conn)->client==*maxfd){
        *maxfd=get_max_sock();
    }
    sens_close_connection(*conn);
    sens_destroy_connection(*conn);
    *conn=NULL;

    return RETURN_OK;
}

static int  socket_init(const struct sens_config_t *conf){
    context->sock = socket(AF_INET,SOCK_STREAM, 0);
    if(context->sock==-1){
        #ifndef NDEBUG
            perror("socket");
            LOG_ERROR(context->loghandle,"socket error,check network \n");
        #endif // NDEBUG
        return -1;
    }

    //LOG_INFO(log_writer,"test sdfsd fsdf \n");
    context->addr.sin_family = AF_INET;
    context->addr.sin_port = htons(conf->port);

    context->addr.sin_addr.s_addr = inet_addr(conf->addr);
    if(bind(context->sock,&context->addr,sizeof(struct sockaddr_in))==-1){
        #ifndef NDEBUG
            perror("bind");
            LOG_ERROR(context->loghandle,"socket bind error,addr(%s:%d) \n",conf->addr,conf->port);
        #endif // NDEBUG
        CLOSE_SOCK(context->sock);
        return -1;
    }
    if(listen(context->sock,conf->connection_size)==-1){
        #ifndef NDEBUG
            perror("listen");
            LOG_ERROR(context->loghandle,"socket listen:(%d) \n",conf->connection_size);
        #endif // NDEBUG
        return -1;
    }
    struct timeval rec_timeout;
    rec_timeout.tv_sec=conf->recv_timeout/1000;
    rec_timeout.tv_usec=(conf->recv_timeout%1000)*1000;
    struct timeval send_timeout;
    send_timeout.tv_sec=conf->send_timeout/1000;
    send_timeout.tv_usec=(conf->send_timeout%1000)*1000;
    setsockopt(context->sock,SOL_SOCKET,SO_RCVTIMEO,(void *)&rec_timeout,sizeof(struct timeval));
    setsockopt(context->sock,SOL_SOCKET,SO_SNDTIMEO,(void *)&send_timeout,sizeof(struct timeval));
    return RETURN_OK;
}
int sens_send(const struct sens_connection_t *conn,struct sens_msg_head_t *head,void *body,size_t body_size){

    size_t headlen=sizeof(sens_msg_head);
    int datalen=headlen+body_size;
    if(send(conn->client,(void *)&datalen,4,0)==-1){
        return -1;
    }
    if(send(conn->client,(void *)head,headlen,0)==-1){
        return -1;
    }
    if(body_size==0||body==NULL){
        return RETURN_OK;
    }
    if(send(conn->client,body,body_size,0)==-1){
        return -1;
    }
    return RETURN_OK;
}

int sens_init(const struct sens_config_t *conf){
    context=(struct sens_context_t *)calloc(sizeof(struct sens_context_t),1);
    context->max_connection_size=conf->connection_size;
    context->connections=(struct sens_connection_t **)calloc(sizeof(struct sens_connection_t *),context->max_connection_size);
    context->loghandle=conf->loghnd;
    context->auth_type=conf->auth_type;
    context->closed_flag=0;
    context->connect_handler=conf->connect_handler;
    context->disconnect_handler=conf->disconnect_handler;
    context->login_handler=conf->login_handler;
    context->logout_handler=conf->logout_handler;
    context->message_handler=conf->message_handler;
    #ifdef PTHREADS
    assert(pthread_mutex_init(&context->connection_mutex,NULL)==0);
    #endif // PTHREADS

    if(socket_init(conf)!=RETURN_OK){
        free(context);
        return -1;
    }
    return RETURN_OK;

}
int sens_send_error(SOCKET client,short msg_type){

    sens_msg_head head={0};
    head.msg_type=msg_type;
    head.keepalive=CONNECT_CLOSED;
    head.need_resp=0;
    head.version.ver=SERVER_VERSION;
    return send(client,&head,sizeof(sens_msg_head),0);
}
int sens_send_login_params(struct sens_connection_t *conn){
    random_access_code(conn->access_code,ID_STR_LENGTH);
    sens_msg_head head={0};
    head.msg_type=SENS_S_MSG_LOGIN_PARAMS;
    head.keepalive=CONNECT_KEEPALIVED;
    head.need_resp=0;
    head.version.ver=SERVER_VERSION;
    struct sens_msg_accessinfo_t access_info={0};
    memcpy(access_info.access_code,conn->access_code,ID_STR_LENGTH);
    access_info.auth_type=context->auth_type;
    time(&access_info.sys_datetime);

    return sens_send(conn,&head,(void *)&access_info,sizeof(struct sens_msg_accessinfo_t));
}
int sens_msg_handle(struct sens_connection_t *conn,const struct sens_msg_head_t *head,char *body,size_t body_size){

    if(head->msg_type==SENS_CLIENT_MSG_LOGIN){
        struct sens_msg_login_t *loginfo=(struct sens_msg_login_t *)body;
        sens_msg_login_resp resp={0};
        resp.login_flag=context->login_handler(conn,head,loginfo);

        if(resp.login_flag==LOGIN_OK){
            conn->logined=1;
        }
        else{
            conn->logined=0;
            random_access_code(conn->access_code,ID_STR_LENGTH);
            memcpy(resp.access_code,conn->access_code,ID_STR_LENGTH);

        }
        sens_msg_head head={0};
        head.msg_type=SENS_S_MSG_LOGIN_RESP;
        head.keepalive=CONNECT_KEEPALIVED;
        head.need_resp=0;
        head.version.ver=SERVER_VERSION;
        return sens_send(conn,&head,(void *)&loginfo,sizeof(struct sens_msg_login_t));


    }
    else if(head->msg_type==SENS_CLIENT_MSG_BEAT){
        sens_msg_head head={0};
        head.msg_type=SENS_S_MSG_BEAT_RESP;
        head.keepalive=CONNECT_KEEPALIVED;
        head.need_resp=0;
        head.version.ver=SERVER_VERSION;
        return sens_send(conn,&head,NULL,0);

    }
    else if(head->msg_type==SENS_CLIENT_MSG_LOGOUT){
        context->logout_handler(conn);
    }
    else{
        if(conn->logined){
            return context->message_handler(conn,head,body,body_size);
        }
        else{
            sens_msg_head head={0};
            head.msg_type=SENS_S_MSG_LOGIN_REQUIRE;
            head.keepalive=CONNECT_KEEPALIVED;
            head.need_resp=0;
            head.version.ver=SERVER_VERSION;
            return sens_send(conn,&head,NULL,0);
        }

    }

}
void *sens_running(void *flag){

    int *closed_flag=(int *)flag;
    fd_set socket_set[2];

    FD_ZERO(&socket_set[0]);
    FD_SET(context->sock,&socket_set[0]);

    SOCKET max_sock_fd=context->sock;

    struct timeval interval;
    interval.tv_sec=SELECT_TIMEOUT/1000000;
    interval.tv_usec=SELECT_TIMEOUT%1000000;
    size_t socket_addr_len=sizeof(struct sockaddr_in);
    while(!(*closed_flag)){
        socket_set[1]=socket_set[0];
        int ret=select(max_sock_fd+1,&socket_set[1],NULL,NULL,&interval);
        if(ret<0){

            *closed_flag=1;
            #ifdef PTHREADS
            pthread_exit(closed_flag);
            #endif // PTHREADS
            LOG_ERROR(context->loghandle,"select error \n");
            return closed_flag;
        }
        else if(ret==0){
                continue;
            //select timtout
        }
        else{
            if(FD_ISSET(context->sock,&socket_set[1])){
                //SOCKET client=accept()
                //struct sens_connection_t *conn=NULL;
                struct sens_connection_t **refconn=sens_create_connection();
                if(refconn!=NULL){
                    struct sens_connection_t *conn=*refconn;
                    conn->client=accept(context->sock,&conn->client_addr,&socket_addr_len);

                    FD_SET(conn->client,&socket_set[0]);
                    if(conn->client>max_sock_fd){
                        max_sock_fd=conn->client;
                    }

                    #ifndef NDEBUG
                    //char buff[32];
                    printf("client %s:%d connected \n",inet_ntoa(conn->client_addr.sin_addr),ntohs(conn->client_addr.sin_port));
                    #endif // NDEBUG
                    time(&conn->last_active_time);
                    if(context->connect_handler(conn)==CONNECT_ALLOW){
                        if(sens_send_login_params(conn)!=RETURN_OK){
                            remove_connection(refconn,&socket_set[0],&max_sock_fd);

                        }
                    }
                    else{
                        sens_send_error(conn->client,SENS_S_MSG_ACCESS_DENIED);
                        remove_connection(refconn,&socket_set[0],&max_sock_fd);
                    }



                }


            }
            for(size_t i=0;i<context->max_connection_size;i++){

                if(context->connections[i]==NULL)
                    continue;
                struct sens_connection_t **conn=&context->connections[i];
                if(FD_ISSET((*conn)->client,&socket_set[1])){
                    //char buff[4];
                    unsigned int datalen=0;

                    ssize_t recvsize=recv((*conn)->client,(void *)&datalen,sizeof(unsigned int),MSG_WAITALL);
                    //printf("datalen %d \n",datalen);
                    if(recvsize==0){

                        remove_connection(conn,&socket_set[0],&max_sock_fd);
                        //if(context->connections[i]==NULL)
                        //    printf("conn is null,closed \n");

                    }
                    else if(recvsize==-1){
                        #ifndef NDEBUG
                        perror("recv");
                        #endif // NDEBUG
                        remove_connection(conn,&socket_set[0],&max_sock_fd);
                    }
                    else{
                        if(datalen>MAX_BUFFER_SIZE){
                            sens_send_error((*conn)->client,SENS_S_MSG_DATA_RESOLVE_ERROR);


                            LOG_WARN(context->loghandle,"data resolve error,client[%s,%d] \n",
                                inet_ntoa((*conn)->client_addr.sin_addr),ntohs((*conn)->client_addr.sin_port));
                            remove_connection(conn,&socket_set[0],&max_sock_fd);


                        }
                        else{
                            char *buffer=(char *)malloc(datalen);
                            size_t headlen=sizeof(sens_msg_head);
                            recvsize=recv((*conn)->client,buffer,datalen,MSG_WAITALL);
                            if(recvsize==0){
                                remove_connection(conn,&socket_set[0],&max_sock_fd);
                            }
                            else if(recvsize==-1){
                                #ifndef NDEBUG
                                perror("recv");
                                #endif // NDEBUG
                                //strerror(errno);
                                remove_connection(conn,&socket_set[0],&max_sock_fd);
                            }
                            else if(recvsize<headlen){
                                LOG_WARN(context->loghandle,"data resolve error,client[%s,%d] \n",
                                inet_ntoa((*conn)->client_addr.sin_addr),ntohs((*conn)->client_addr.sin_port));
                                remove_connection(conn,&socket_set[0],&max_sock_fd);
                            }
                            else{
                                sens_msg_head head={0};
                                memcpy(&head,buffer,headlen);
                                char *body=buffer+headlen;
                                size_t bodylen=datalen-headlen;
                                time(&(*conn)->last_active_time);
                                if(sens_msg_handle(*conn,&head,bodylen,body)!=RETURN_OK){
                                    remove_connection(conn,&socket_set[0],&max_sock_fd);
                                }
                            }
                            free(buffer);
                        }
                    }


                }
            }

        }


    }
    *closed_flag=1;
    return closed_flag;

}
int sens_start(){

    #ifdef PTHREADS
    if(pthread_create(&context->run_pid, NULL, (void*)sens_running, &context->closed_flag)==-1){
        LOG_SYS(context->loghandle,"pthread create error \n");
        return -1;
    }
    //ASSERT_ERROR(error_info!=-1,"accept thread create error",INERNAL_ERROR);
    #else
        sens_running(&context->closed_flag);
    #endif // PTHREADS
    LOG_INFO(context->loghandle,"sensor server start success ! \n");
    return RETURN_OK;

}
struct sens_connection_t **sens_create_connection(){
    if(context->max_connection_size<=context->cur_connection_count){
        return -1;
    }
    for(size_t i=0;i<context->max_connection_size;i++){
        if(context->connections[i]==NULL){
            context->connections[i]=calloc(sizeof(struct sens_connection_t),1);
            context->cur_connection_count++;
            //*conn=context->connections[i];
            return &context->connections[i];
        }
    }
    return NULL;
}
int sens_close_connection(struct sens_connection_t *conn){
    if(conn->client<=0)
        return -1;
    shutdown(conn->client,SHUT_RDWR);
    CLOSE_SOCK(conn->client);

    return RETURN_OK;

}
int sens_destroy_connection(struct sens_connection_t *conn){
    if(conn==NULL)
        return -1;
    free(conn);
    context->cur_connection_count--;
    return RETURN_OK;
}
int sens_destroy(){

    shutdown(context->sock,SHUT_RDWR);
    CLOSE_SOCK(context->sock);
    context->closed_flag=1;
    #ifdef PTHREADS
    void *exitcode=NULL;
    pthread_join(context->run_pid,&exitcode);
    #endif // PTHREADS
    //printf("sens destory \n");

    for(size_t i=0;i<context->max_connection_size;i++){

        if(context->connections[i]!=NULL){
            sens_close_connection(context->connections[i]);
            free(context->connections[i]);
            context->connections[i]=NULL;
        }

    }
    context->cur_connection_count=0;
    free(context->connections);
    free(context);
    return RETURN_OK;
}
