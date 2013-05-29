#include "network.h"

network::network()
{
    sockfd = -1;
    pthread_mutex_init(&mutex, NULL);
}

network::~network()
{
    close(sockfd);
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


	ret = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (ret < 0) {
	    //连接请求
		printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
	}

    //printf("connect to server success\n");

	return 0;
}

int network::send_buff(void* buff)
{

    return 0;
}


int network::recv_buff(void* buff)
{
    int ret = -1;

    //char buff[640*480*3/2];
    memset(buff, 0, 640*480*2);
    //memset(buff, 0, 40);

    init_client();
    //printf("recv buff is start\n");
    for (int i = 0; i < 600; i++)
        recv(sockfd, buff + i*1024, 1024, 0);
    
    //recv(sockfd, buff, 640*480*2, 0);

    //printf("recv buff is ok\n");
    //printf("buff:%s\n", buff);
    close(sockfd);
    return 0;
}

int network::get_network_fd()
{
    return sockfd;
}
