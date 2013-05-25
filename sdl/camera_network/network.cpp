#include "network.h"

network::network()
{
    listenfd = -1;
    connfd = -1;
}

network::~network()
{

}


int network::init_server()
{
    int ret = -1;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
		printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
	}

	memset(&clientaddr, 0, sizeof(clientaddr));
    clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	clientaddr.sin_port = htons(6666);//

    ret = bind(listenfd, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
    if (ret < 0) {
		printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
    }

    ret = listen(listenfd, 10);
    if (ret < 0) {
		printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
		exit(0);
    }

	printf("======waiting for client's request======\n");


    return 0;
}

int network::init_client()
{
    int ret = -1;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
	    // 创建套接字描述符给sockfd
		printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
		exit(0);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	//结构体清零
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(6666);//端口号
	if( inet_pton(AF_INET, "192.168.66.69", &servaddr.sin_addr) <= 0) {
	//Linux下IP地址转换函数，可以在将IP地址在“点分十进制”和“整数”之间转换 
		printf("inet_pton error for \n");
		exit(0);
	}



    printf("connect to server success\n");

	return 0;
}

int network::send_buff(void* buff)
{
    int ret = -1;
    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
    if (connfd < 0) {
	    printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
	    return -1;	
	}

    ret = send(connfd, (const void *)buff, 640*480*3/2, 0);
    if (ret < 0) {
		printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
    }

    return 0;
}


int network::recv_buff(void* buff)
{
    int ret = -1;

	ret = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (ret < 0) {
	    //连接请求
		printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
	}
    //char buff[640*480*3/2];
    memset(buff, 0, 640*480*2);

    for (int i = 0; i < 600; i++)
        recv(sockfd, buff + i*1024, 1024, 0);
    //printf("buff %s\n", buff);

    close(sockfd);
    return 0;
}

int network::get_network_fd()
{
    return sockfd;
}


#if 0
#if 0
int main()
{
    char p[40];
    memset(p, 0, 40);
    strncpy(p, "hello world", 20);

    network serv;
    serv.init_server();
    while (1) {
        serv.send_buff((void *)p);
        sleep(1);
    }
}
#else
int main()
{
    network client;
    client.init_client();
    //while (1) {
    client.recv_buff();
        sleep(1);
    //}
}

#endif
#endif
