cmake_minimum_required(VERSION 3.5)
project(my_netlib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(SOURCE_FILES
        FDB_Epoll.cpp
        FDB_Epoll.h
    main.cpp FDB_Users.cpp FDB_Users.h FDB_Accept.cpp FDB_Accept.h FDB_Socket.cpp FDB_Socket.h FDB_Server.cpp FDB_Server.h)

add_executable(my_netlib ${SOURCE_FILES})