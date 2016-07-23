#include <iostream>
#include<mynet.h>
using namespace std;
#define BUF_SIZE 1024
#define LISTEN_MAX 1000

int main() {
    cout << "TCP select 回射服务器!" << endl;

    int maxi, maxfd, listenfd, connfd, sockfd, num, i;
    int nready, client[FD_SETSIZE];
    ssize_t n;

    fd_set rset, allset;
    socklen_t cli_len;
    struct sockaddr_in cliaddr, servaddr;

    char buf[BUF_SIZE];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int resuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &resuse, sizeof(resuse));

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(10086);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, LISTEN_MAX);

    maxfd = listenfd;
    maxi = -1;
    for(int i = 0; i < FD_SETSIZE; i++)
    {
        client[i] = -1;
    }

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    while(1)
    {
        rset = allset;
        printf("selecting\n");

        nready = select(maxfd+1, &rset, NULL, NULL, NULL);
        printf("%d\n",nready);
        printf("selected\n");
        if(FD_ISSET(listenfd, &rset))
        {

            cli_len = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &cli_len);
            printf("connfd: %d", connfd);
            printf("Get new one client\n");
            for(i = 0; i < maxfd; i++) {
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            }
            FD_SET(connfd, &allset);
            if(connfd > maxfd)
                maxfd = connfd;
            if(i > maxi)
                maxi = i;
            nready --;
            if(nready <= 0)
                continue;
        }
        for(i = 0; i <= maxi; i++)
        {
            if((sockfd = client[i]) < 0)
                continue;
            if( FD_ISSET(sockfd, &rset))
            {
                n = read(sockfd, buf, sizeof(buf));
                if(n == 0)
                {
                    printf("lost a client\n");
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                }
                else{
                    write(sockfd, buf, n);
                }
                nready --;
                if(nready <= 0)
                    break;
            }
        }


    }


    return 0;
}
