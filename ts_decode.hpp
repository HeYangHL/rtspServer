#ifndef _TS_DECODE_HPP_
#define _TS_DECODE_HPP_
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <fstream>
#include <list>
#include <sys/time.h>
#include <sys/select.h>
#include "ts_decoder.hpp"
#include "mutex.hpp"
#include "thread_rtsp.hpp"
#include "Def.h"

class Frame_t{
public:
    uint64_t pts = 0;
    uint32_t size = 0;
    uint8_t *frame = NULL;
};

class UDP_TS:public THREAD_RTSP
{
public:
    UDP_TS()
    {
        fd = 0;
    }
    ~UDP_TS()
    {
        std::list<Frame_t *>::iterator iter;
        std::list<Frame_t *>::iterator it;

        if(!_v_list.empty())
        {
            for(iter = _v_list.begin(); iter != _v_list.end(); iter++)
            {
                if((*iter)->frame)
                {
                    delete [] (*iter)->frame;
                    (*iter)->frame = NULL;
                }
                _v_list.erase(iter);
                iter--;
            }
        }
        if(!_a_list.empty())
        {
            for(it = _a_list.begin(); it != _a_list.end(); it++)
            {
                if((*it)->frame)
                {
                    delete [] (*it)->frame;
                    (*it)->frame = NULL;
                }
                _a_list.erase(it);
                it--;
            }
        }
    }
    void create_udp(std::string ip, int port);
    void bind_udp(std::string ip, int port);
    int write_video(unsigned char* data, int size, uint64_t pts, int64_t ms);
    int read_video(unsigned char* data,int* size, uint64_t* pts, int64_t ms);
    int write_h264_video(unsigned char* data, int size, uint64_t pts, int64_t ms);
    int read_h264_video(unsigned char* data,int* size, uint64_t* pts, int64_t ms);
    int write_audio(unsigned char* data, int size, uint64_t pts, int64_t ms);
    int read_audio(unsigned char* data,int* size, uint64_t* pts, int64_t ms);
    int write_h264_audio(unsigned char* data, int size, uint64_t pts, int64_t ms);
    int read_h264_audio(unsigned char* data,int* size, uint64_t* pts, int64_t ms);
    static std::list<Frame_t *> _v_list;
    static std::list<Frame_t *> _a_list;
    static std::list<Frame_t *> _v_h264_list;
    static std::list<Frame_t *> _a_h264_list;
    virtual void thread_proc(void);
    void start_decode(void);
private:
    int fd = 0;;
    int fd1 = 0;
    struct sockaddr_in myaddr;
    struct sockaddr_in myaddrc;
    TsDecoder ts_decode;
    MUTEX ts_mutex;
    MUTEX h264_mutex;
};

#endif