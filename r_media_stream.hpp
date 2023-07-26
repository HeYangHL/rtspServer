#ifndef _R_MEDIA_STREAM_HPP_
#define _R_MEDIA_STREAM_HPP_
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "Def.h"
#include "thread_rtsp.hpp"
#include "sock.hpp"

class MEDIA_STREAM:public THREAD_RTSP
{
public:
    MEDIA_STREAM(){};
    ~MEDIA_STREAM(){};
    //change by yh 2022/1/17
    int getFrameOfVideo(unsigned char* frame, std::string media_source, uint64_t *v_pts);
    int getFrameOfAudio(unsigned char* frame, std::string media_source, uint64_t *a_pts);
    int startCode3(unsigned char* buf);
    int startCode4(unsigned char* buf);
    int isSepChar(unsigned char* buf);
    uint8_t* findNextStartCode(unsigned char* buf, int len);
    
    void start_media(void);
    virtual void thread_proc(void);
    int rtpSendH264Frame(SOCK_T *sock, RtpHdr* rtpPacket, uint8_t* frame, uint32_t frameSize, uint32_t &RtcpChar);
    int rtpSendH265Frame(SOCK_T *sock, RtpHdr* rtpPacket, uint8_t* frame, uint32_t frameSize, uint32_t &RtcpChar);
    bool IsAccessSeparator(uint8_t* frame, int framesize, int &len, std::string mtype);
    int rtpSendPacket(SOCK_T *m_sock, std::string type, uint8_t *frame_data, uint32_t dataSize);
    void net_to_host(RtpHdr* rtpPacket);
    void host_to_net(RtpHdr* rtpPacket);
    void rtcp_host_to_net(RtcpHdr* rtcpPacket);
    void rtcp_net_to_host(RtcpHdr* rtcpPacket);
    int four_payload(uint8_t *buf, int len);
    int rtpSendAACFrame(SOCK_T *sock, RtpHdr* rtpPacket, uint8_t* frame, uint32_t frameSize, uint32_t &RtcpChar);
    int v_rtp_head_init(RtpHdr *rtp_head, uint32_t ssrc);
    int a_rtp_head_init(RtpHdr *rtp_head, uint32_t ssrc);
    int v_rtcp_head_init(RtcpHdr *rtcp_head, uint32_t ssrc);
    int a_rtcp_head_init(RtcpHdr *rtcp_head, uint32_t ssrc);
    int get_adts_bye(uint8_t *frame);
    int rtcpSendControl(SOCK_T *sock, std::string type, RtcpHdr v_rtcpPacket, uint64_t pNum, uint64_t pts, uint64_t pLen);
    int rtcpSendPacket(SOCK_T *m_sock, std::string type, uint8_t *frame_data, uint32_t dataSize);

private:
    
    std::string source_name;
    
};



#endif