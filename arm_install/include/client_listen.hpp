#ifndef _CLIENT_LISTEN_HPP_
#define _CLIENT_LISTEN_HPP_

#include <iostream>
#include <stdio.h>
#include <list>
#include <netinet/tcp.h>
#include "thread_rtsp.hpp"

class CLI_LIST:public THREAD_RTSP{
public:
    virtual void thread_proc(void);
    void start_client_listen();

};


#endif