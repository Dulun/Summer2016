/*************************************************************************
        > File Name: server2.cpp
      > Author: dulun
      > Mail: dulun@xiyoulinux.org
      > Created Time: 2016年07月20日 星期三 09时12分28秒
 ************************************************************************/

#include<iostream>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<algorithm>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<errno.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<pthread.h>

#define MAX_EVENT_NUMBER 1024
#define TCP_BUFFER_SIZE 512
#define UDP_BUFFER_SIZE 1024

int setnonblocking(int fd)
{
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_opt);
    return old_opt;

}

void addfd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

int main()
{
    const char * ip = "127.0.0.1";
    int port = 10086;
    int ret = 0;
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    int resuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &resuse, sizeof(resuse));


    ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    int udpfd = socket(PF_INET, SOCK_DGRAM, 0);
    assert(udpfd >= 0);

    ret = bind(udpfd, (struct sockaddr *)&address, sizeof(address));
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);

    addfd(epollfd, listenfd);
    addfd(epollfd, udpfd);


    while(1)
    {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if(number < 0)
        {
            printf("epoll failed");
            break;
        }
        for(int i = 0; i < number; i++)
        {
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrelength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrelength);
                addfd(epollfd, connfd);
            }
            else if(sockfd == udpfd)
            {
                char buf[UDP_BUFFER_SIZE];
                memset(buf, 0, sizeof(buf));
                struct sockaddr_in client_address;
                socklen_t client_addresslength = sizeof(client_address);
                ret = recvfrom(udpfd, buf, UDP_BUFFER_SIZE-1, 0, (struct sockaddr*)&client_address, &client_addresslength);
                if(ret > 0) {
                    sendto(udpfd, buf, UDP_BUFFER_SIZE - 1, 0, (struct sockaddr *) &client_address, client_addresslength);
                }
            }
            else if(events[i].events & EPOLLIN)
            {
                char buf[TCP_BUFFER_SIZE];
                while(1)
                {
                    memset(buf, 0, sizeof(buf));
                    ret = recv(sockfd, buf, TCP_BUFFER_SIZE - 1, 0);
                    if(ret < 0)
                    {
                        if(errno == EAGAIN || errno == EWOULDBLOCK)
                        {
                            break;
                        }
                        close(sockfd);
                        break;
                    }
                    else if(ret == 0)
                    {
                        close(sockfd);
                    }
                    else
                    {
                        send(sockfd, buf, ret, 0);//回射
                    }
                }
            }
            else{
                printf("somethin else happened \n");
            }
        }
    }
    close(listenfd);


    return 0;
}
