#include "network.h"

network::network()
{
    listenfd = -1;
    connfd = -1;
    pthread_mutex_init(&mutex, NULL);
}

network::~network()
{
    close(listenfd);
    pthread_mutex_destroy(&mutex);
}

void network::m_lock()
{
    pthread_mutex_lock(&mutex);
}

void network::m_unlock()
{
    pthread_mutex_unlock(&mutex);
}

int network::init_server()
{
    int ret = -1;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
		printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
	}

	bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(6666);//
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (ret < 0) {
		printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
    }

    ret = listen(listenfd, 1);
    if (ret < 0) {
		printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
    }

	printf("======waiting for client's request======\n");

    return 0;
}

int network::init_client()
{
	return 0;
}

int network::send_buff(void* buff)
{
    int ret = -1;
    ret = sizeof(clientaddr);

    connfd = accept(listenfd, (struct sockaddr*)&clientaddr, (socklen_t*)&ret);
    if (connfd < 0) {
	    printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
	    return -1;	
	}

    for (int i = 0; i < 600; i++) {
        ret = send(connfd, (const void *)(buff + (i*1024)), 1024, 0);
        if (ret < 0) {
            printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
            exit(0);
        }
    }

    close(connfd);
    return 0;
}


int network::recv_buff()
{
    return 0;
}
