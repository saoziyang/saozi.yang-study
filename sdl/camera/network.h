#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ortp/ortp.h>
#include <signal.h>
#include <pthread.h>



class network {
private:
    int listenfd;
    int connfd;
    int sockfd;
	struct sockaddr_in  servaddr;
	struct sockaddr_in  clientaddr;
	RtpSession *session;
    unsigned int user_ts;
    pthread_mutex_t mutex;
public:
    network();
    ~network();
    //void m_init_rtp_session();
    int init_server();
    int init_client();
    void m_lock();
    void m_unlock();
    int send_buff(void* buff); 
    int recv_buff();
};
