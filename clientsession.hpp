#ifndef _CLIENTSESSION_HPP_
#define _CLIENTSESSION_HPP_

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "thread_rtsp.hpp"
#include "Def.h"
#include "sock.hpp"
#include "sdp.hpp"


class RTSP_SESSION : public THREAD_RTSP
{
public:
    RTSP_SESSION(){};
    ~RTSP_SESSION(){};
    void start(int fd, uint8_t *ip);
    int recv_data(void);
    int handle_data(const char *data);
    int get_method(const char *data);
    void get_str(const char *data, const char *s_mark, const char *e_mark, char *dest, int dest_len);
    int get_cseq(const char *data);
    int get_url(const char *data);
    int get_version(const char *data);
    void handle_options(void);
    void handle_describe(void);
    void handle_setup(const char *data);
    void handle_play(const char *data);
    void handle_pause(void);
    void handle_set_parameter(void);
    void handle_get_parameter(void);
    void send_cmd(unsigned char *cmd, int cmd_len);
    void send_simple_cmd(int coed);
    uint64_t GetTime();
    void init_file(FILE* file);
    void remove_space(char *buf, int len);
    void handle_teardown(void);
    int get_cli_rtp_port(){return clientRtpPort;}
    int get_cli_rtcp_port(){return clientRtcpPort;}
    int get_video_rtp_port(){return video_rtpport;}
    SOCK_T *get_socket(void){return &sock;}
    int clear_source(int fd);
    std::string get_source_name(){return source_name;}
    int isPlayState(){return isPlay;}
    int set_keeplive(void);
    static bool _play;

private:
    virtual void thread_proc(void);
    SOCK_T sock;
    SDP_M sdp;
    int _method = 0;
    char _CSeq[128] = {0};
    char _rtsp_url[256] = {0};
    char _rtsp_ver[128] = {0};
    char _session[128] = {0};
    int m_rtp_ch = 0;
    uint64_t m_range = 0;
    int clientRtpPort = 0;
    int clientRtcpPort = 0;
    uint64_t _video_session;
    uint64_t _audio_session;
    int video_rtpport = 0;
    int video_rtcpport = 0;
    int audio_rtpport = 0;
    int audio_rtcpport = 0;
    int isPlay = 0;
//    int clientfd = 0;
    std::string source_name;
};

#endif