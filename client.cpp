/*************************************************************************
        > File Name: client.c
      > Author: dulun
      > Mail: dulun@xiyoulinux.org
      > Created Time: 2016年07月19日 星期二 11时01分09秒
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<poll.h>
#include<fcntl.h>
#include<assert.h>
#define BUFFER_SIZE 64

int main()
{
   // const char * ip = "115.159.53.185";
    const char * ip = "127.0.0.1";
    //int port = 6550;
    int port = 10086;

    struct sockaddr_in server_address;
    bzero( &server_address, sizeof(server_address) );
    server_address.sin_family = AF_INET;
    inet_pton( AF_INET, ip, &server_address.sin_addr );
    server_address.sin_port = htons(port);

    int sockfd = socket( PF_INET, SOCK_STREAM, 0 );
    assert(sockfd >= 0);

    if( connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0 )
    {
        printf("connetcion failed\n");
        close(sockfd);
        return 1;
    }
    printf("connetc over!\n");

    pollfd fds[2];  //0：标准输入
    fds[0].fd = 0;
    fds[0].events = POLLIN; //可读事件
    fds[0].revents = 0;     //内核处理

    //1：套接字描述符
    fds[1].fd = sockfd;
    fds[1].events = POLLIN | POLLRDHUP; //可读或对方中断
    fds[1].revents = 0;

    char read_buf[BUFFER_SIZE]; //读缓存, 用于接收
    int pipefd[2];
    int ret = pipe(pipefd);
    assert(ret != -1);

    while(1)
    {
        ret = poll(fds, 2, -1);  //阻塞，监听两个感兴趣的fd
        if(ret < 0)
        {
            printf("poll failed\n");
            break;
        }

        if(fds[1].revents & POLLRDHUP)
        {
            printf("server close the connetcion\n");
            break;
        }
        else if( fds[1].revents & POLLIN )
        {
            memset( read_buf, 0, BUFFER_SIZE );
            recv(fds[1].fd, read_buf, BUFFER_SIZE - 1, 0);
            printf("%s\n", read_buf);
        }

        if(fds[0].revents & POLLIN)
        {
            //标准输入输出 通过管道 。去sockfd
            //从标准输入读 ，进管道， 大小32768 更多或文件直接走内核缓冲区
            ret = splice( 0, NULL, pipefd[1], NULL, 32768 , SPLICE_F_MORE | SPLICE_F_MOVE);  //  pipefd[0] 读
            //从管道读， 进套接字描述符， 大小32768 更多或文件直接走内核缓冲区
            ret = splice( pipefd[0], NULL, sockfd, NULL, 32768 , SPLICE_F_MORE | SPLICE_F_MOVE); //写
        }
    }
    close(sockfd);
    return 0;
}
