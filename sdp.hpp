#ifndef _SDP_HPP_
#define _SDP_HPP_

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>

#include "sock.hpp"
#include "r_media_stream.hpp"
#include "thread_rtsp.hpp"
#include "Def.h"
#include "mutex.hpp"

class SDP_M:public THREAD_RTSP
{
public:
    int init_sdp(const char *content, int cli_fd);
    const char *get_sdp(void){return m_sdp;}
//    virtual void thread_proc(void);
//    int play(SOCK_T *sock, int rtp_ch);
//    void display(void);

    char *get_track_id(){return track_id;}
    uint16_t get_seq(){return seq;}
    uint64_t get_rtp_time(){return rtp_time;}
private:
    char m_sdp[2048] = {0};
//    MEDIA_STREAM media_stream;

    int m_rtp_ch = 0;
//    SOCK_T *m_sock = NULL;
    char track_id[32] = {0};
    uint16_t seq = 0;
    uint64_t rtp_time = 0;
    std::string rtsp_source_name;
};


#endif
