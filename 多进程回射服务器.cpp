#include <iostream>
#include<mynet.h>
#define SA struct sockaddr
using namespace std;

void str_echo(int connfd)
{
    ssize_t n;
    char buf[100];
    printf("back to new client\n");
    while((n = read(connfd, buf, 100)) > 0)
    {
        if(n == 0)
        {
            printf("one client offline!\n");
            exit(0);
        }
        write(connfd, buf, n-1);
    }
}

int main() {
    cout << "TCP 多进程回射服务器demo!" << endl;
    const char * ip = "127.0.0.1";
    int       listenfd, connfd;
    pid_t     childpid;
    socklen_t cli_len;
    struct sockaddr_in cli_addr, serv_addr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int resuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &resuse, sizeof(resuse));

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);
    serv_addr.sin_port = htons(10086);

    int ret;
    ret = bind(listenfd,(SA*)&serv_addr , sizeof(serv_addr));
    assert(ret != -1);
    listen(listenfd, 100);

    while(1)
    {
        cli_len = sizeof(cli_addr);
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &cli_len);
        printf("accepted\n");
        if( (childpid = fork() ) == 0) //子进程
        {
            close(listenfd);
            str_echo(connfd);
            exit(0);
        }
        close(connfd);
    }
    return 0;
}