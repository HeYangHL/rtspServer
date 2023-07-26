#ifndef _SOCK_HPP_
#define _SOCK_HPP_

#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
//#include <linux/tcp.h>
#include <netinet/tcp.h>
#include <map>
#include <sstream>
#include <fstream>
#include "mutex.hpp"
#include "thread_rtsp.hpp"
#include "Def.h"


class RTSP_S;

class SOCK_T:public THREAD_RTSP
{
public:
    SOCK_T(){};
    ~SOCK_T(){
        if(tcp_pid > 0)
            pthread_cancel(tcp_pid);
    };

    int open_sock(void);
    int set_sock_opt(void);
    int bind_sock(void);
    int listen_sock(void);
    void set_tcp_fd(int fd){tcp_fd = fd; return;}
    int get_tcp_fd(){return tcp_fd;};
    int Recv(const unsigned char *buf, int len);
    int Send(const unsigned char* buf, int len);
    int SendTo(std::string type, const unsigned char* buf, int len);
    int SendRtcpTo(std::string type, const unsigned char* buf, int len);
    int RecvFrom(const unsigned char* buf, int len);
    int createRtpSocket();
    int createRtcpSocket();
    int bindRtcpSocketAddr();
    int bindRtpSocketAddr();
    int createAudioRtpSocket();
    int createAudioRtcpSocket();
    int bindAudioRtpSocketAddr();
    int bindAudioRtcpSocketAddr();
    static int serverRtpSockfd;
    static int serverRtcpSockfd;
    static int serverAudioRtpSockfd;
    static int serverAudioRtcpSockfd;

    int set_video_rtp_port(int port){videoRtpPort = port;}
    int set_video_rtcp_port(int port){videoRtcpPort = port;}
    int set_audio_rtp_port(int port){audioRtpPort = port;}
    int set_audio_rtcp_port(int port){audioRtcpPort = port;}
    int set_client_ip(uint8_t *ip){memcpy(client_ip, ip, sizeof(client_ip));}
    void sock_set_pthread_des_stat(bool val){
        set_pthrea_des_stat(val);
    }

private:
    int tcp_fd = 0;  
    int clientPort = 0;
    int clientRtpPort = 0;
    int clientRtcpPort = 0;
    int videoRtpPort = 0;
    int videoRtcpPort = 0;
    int audioRtpPort = 0;
    int audioRtcpPort = 0;
    uint8_t client_ip[16] = {0};
    std::string server_ip = "0.0.0.0";
    unsigned short server_port = 8554;
    pthread_t tcp_pid = 0;
    
    virtual void thread_proc(void);
    virtual void accept_sock(int c_fd, uint8_t *cli_ip){return ;}
};

#endif
