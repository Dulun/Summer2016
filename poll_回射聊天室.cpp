/*************************************************************************
        > File Name: server.cpp
      > Author: dulun
      > Mail: dulun@xiyoulinux.org
      > Created Time: 2016年07月19日 星期二 11时26分40秒
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
#include<fcntl.h>
#include<poll.h>
#include<errno.h>
#include<unistd.h>
using namespace std;

#define USER_LIMIT 100
#define BUFFER_SIZE 64
#define FD_LIMIT 65535

struct client_data
{
    sockaddr_in address;
    char * write_buff;
    char buf[BUFFER_SIZE];
};

int setnonblocking( int fd )
{
    int old_option = fcntl( fd, F_GETFL );
    int new_option = old_option | O_NONBLOCK;
    fcntl( fd, F_SETFL, new_option );
    return old_option;
}

int main()
{
    const char *ip = "127.0.0.1";
    int port = 10086;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    int resuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &resuse, sizeof(resuse));
    assert(listenfd >= 0);

    int ret;
    ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address)); //命名套接字。
    assert(ret != -1);

    ret = listen(listenfd, USER_LIMIT);  //监听
    assert(ret != -1);


    client_data * users = new client_data[FD_LIMIT];

    pollfd fds[USER_LIMIT+1];
    int user_counter = 0;
    for(int i = 1; i <= USER_LIMIT; i++)
    {
        fds[i].fd = -1;
        fds[i].events = 0;
    }

    //初始化第一个
    fds[0].fd = listenfd;
    fds[0].events = POLLIN | POLLERR;
    fds[0].revents = 0;

    while(1)
    {
        ret = poll( fds, user_counter+1, -1 );
        if(ret < 0)
        {
            printf("POLL failed\n");
            break;
        }

        for(int i = 0; i < user_counter+1; i++)
        {
            if((fds[i].fd == listenfd) && (fds[i].revents & POLLIN) )
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_data);
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
                printf("accept\n");
                if(connfd < 0)
                {
                    printf("errno is : %d \n", errno);
                    continue;
                }
                if(user_counter >= USER_LIMIT)
                {
                    const char * info = "too many users\n";
                    printf("%s", info);
                    send(connfd, info, strlen(info), 0);
                    close(connfd);
                    continue;
                }
                user_counter++;
                users[connfd].address = client_address;
                setnonblocking(connfd);
                fds[user_counter].fd = connfd;
                fds[user_counter].events = POLLIN | POLLERR;
                fds[user_counter].revents = 0;
                printf("comes a new user, now have %d users\n", user_counter);
            }
            else if(fds[i].revents & POLLERR) //ERROR  ， fds[i].revent & POLLERR 为真，表示，发生错误,下同
            {
                printf("get an error form %d\n", fds[i].fd);
                char errors[100];
                memset(errors, 0, sizeof(errors));
                socklen_t length = sizeof(errors);
                if( getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, &errors, &length) < 0)
                {
                    printf("get socket option failed\n");
                }
                continue;
            }
            else if(fds[i].revents & POLLRDHUP)
            {
                //users[fds[i].fd] = users[fds[user_counter].fd];
                close(fds[i].fd);
                //fds[i] = fds[]
                printf("a client left\n");
                sleep(1);
            }
            else if(fds[i].revents & POLLIN)
            {
                int connfd = fds[i].fd;
                memset(users[connfd].buf, 0, BUFFER_SIZE);
                ret = recv(connfd, users[connfd].buf, BUFFER_SIZE-1, 0);
                send(connfd, users[connfd].buf, BUFFER_SIZE-1, 0);
                if(ret == 0)
                {
                    close(connfd);
                    printf("one client left\n");
                    user_counter--;
                    i--;
                    continue;
                }
                printf("get %d bytes of client data %s from %d \n", ret, users[connfd].buf, connfd);
                //sleep(10000); //test!
                if(ret < 0)
                {
                    if(errno != EAGAIN)
                    {
                        close(connfd);
                        users[fds[i].fd] = users[fds[user_counter].fd];
                        fds[i] = fds[user_counter];
                        i--;
                        user_counter--;
                    }
                }
                else if( ret == 0 )
                {
                }
                else{
                    for(int j = 1; j <= user_counter; j++)
                    {
                        if(fds[j].fd == connfd) //不等于自己，不给自己设置读写事件
                        {
                            continue;
                        }
                        //fds[j].events |= ~POLLIN; //取消读事件
                        fds[j].events |= POLLOUT; //设置写事件，下次while循环，直接进下一个else if
                        users[fds[j].fd].write_buff = users[connfd].buf;
                    }
                }
            }
            else if(fds[i].revents & POLLOUT) //监听到写事件（次数：user_counter-1）
            {
                int connfd = fds[i].fd;
                if( !users[connfd].write_buff )
                {
                    continue;
                }
                ret = send(connfd, users[connfd].write_buff, strlen(users[connfd].write_buff)-1, 0); //往过发
                users[connfd].write_buff = NULL; //清缓存
               // fds[i].events |= ~POLLOUT; //取消写时间
               // fds[i].events |= POLLIN;   //设置读事件
            }
        }
    }
    delete []users;
    close(listenfd);

    return 0;
}
