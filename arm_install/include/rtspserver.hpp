#ifndef _RTSPSERVER_HPP_
#define _RTSPSERVER_HPP_

#include <iostream>
#include <stdio.h>
#include <list>
#include <map>
#include "sock.hpp"
#include "thread_rtsp.hpp"
#include "clientsession.hpp"
// #include "r_media_stream.hpp"
#include "mutex.hpp"
#include "client_listen.hpp"

/*
*类描述：
*   存放客户端信息及rtsp会话信息
*
*/
class CLI_MSG{
public:
    uint8_t cli_ip[16] = {0};
    int cli_tcp_fd = 0;
    int server_rtp_fd = 0;
    int server_rtcp_fd = 0;
    pthread_t session_pid = 0;
    pthread_t sdp_pid = 0;
    RTSP_SESSION * session_map = NULL;
};

/*
*类描述：
*   添加源的信息
*/

class RTSP_S:public SOCK_T
{
public:
    RTSP_S(){};
    ~RTSP_S()
    {
        #if 0
        std::list<CLI_MSG>::iterator iter;
        std::map<std::string, session>::iterator it;
        for(iter = _f_list.begin(); iter != _f_list.end();iter++)
            _f_list.erase(iter);
        for(it = rtsp_session.begin(); it != rtsp_session.end();it++)
            rtsp_session.erase(it);
        #endif
    }
    
    void start(void);
    void creat_sock(void);
    virtual void accept_sock(int c_fd, uint8_t *cli_ip);
    void add_list(int tcpfd, uint8_t *cli_ip, RTSP_SESSION *rtsp_session);
//    static std::map<int, RTSP_SESSION *> session_map;
    static std::list<CLI_MSG> _f_list;
    
    void creat_media_pthread(void);
    void creat_client_list_pthread(void);

    int add_media_source(std::string sourceName, int (*read_video_stream)(void *opaque, uint8_t *data, bool keyFrame, uint64_t *pts), void *video_opaque, int (*read_audio_stream)(void *opaque, uint8_t *data, int len, uint64_t *pts), void *audio_opaque, uint32_t videoFrameRate, uint32_t audioSampleRate, std::string videoCodecType="h264", std::string audioCodecType="aac");
    int del_media_source(std::string sourceName);
//    int start_rtsp_server();
    int stop();
    static std::map<std::string, SOURCE> rtsp_session;
    static MUTEX mutex;
    static MUTEX source_mutex;
    static MUTEX client_mutex;
    static bool LockFlag;


private:
    // MEDIA_STREAM media_stream;
    CLI_LIST cli_list;

};


#endif