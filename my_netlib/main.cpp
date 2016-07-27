#include <iostream>
#include "FDB_Epoll.h"
#include "FDB_Socket.h"
#include <thread>

static int fd;

void funcA() {
    std::cout << "Server start!" << std::endl;
    FDB_Epoll epo(fd);
    epo.Epoll_wait();

}

int main()
{
    FDB_Socket a(AF_INET, 10);
    fd = a.Socket_getfd();
    std::thread t1(funcA);
    t1.join();
}
