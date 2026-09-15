#ifndef _R_MEDIA_STREAM_HPP_
#define _R_MEDIA_STREAM_HPP_
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <thread>
#include "Def.h"
#include "thread_rtsp.hpp"
#include "sock.hpp"
#include "Fifo_Buffer.hpp"
#include "time_base.hpp"

#define SYNC_THRESHOLD_MS 40


class MEDIA_STREAM:public THREAD_RTSP
{
public:
    MEDIA_STREAM(std::string sourceName, \
                    int (*read_video_stream)(void *opaque, uint8_t *data, bool keyframe, uint64_t *pts), \
                    void *video_opaque, \
                    int (*read_audio_stream)(void *opaque, uint8_t *data, int len, uint64_t *pts), \
                    void *audio_opaque, \
                    uint32_t videoFrameRate, \
                    uint32_t audioSampleRate, \
                    std::string videoCodecType="h264", \
                    std::string audioCodecType="aac") \
                    :source_name(sourceName), \
                    read_video_callback(read_video_stream), \
                    stream_video_opaque(video_opaque), \
                    read_audio_callback(read_audio_stream), \
                    stream_audio_opaque(audio_opaque), \
                    video_fps(videoFrameRate), \
                    audio_sample(audioSampleRate), \
                    video_format(videoCodecType), \
                    audio_format(audioCodecType)

    {};
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
    void audio_stream_play_thread(void);
    void video_stream_play_thread(void);
    void audio_stream_recv_thread(void);
    void video_stream_recv_thread(void);
    uint64_t get_current_time(void);
    uint64_t get_clock(void);
    void set_clock(uint64_t pts);
    uint64_t compute_target_delay(uint64_t duration);
    uint64_t vp_duration(void);
    TimeBase timeBase;

    typedef int (*read_video)(void *opaque, uint8_t *data, bool keyFrame, uint64_t *pts);
    typedef int (*read_audio)(void *opaque, uint8_t *data, int len, uint64_t *pts);
public:
    std::string video_format;
    std::string audio_format;
    uint64_t video_pts;
    uint64_t audio_pts;
    uint32_t video_fps;
    uint32_t audio_sample;
    read_video read_video_callback = nullptr;
    read_audio read_audio_callback = nullptr;
    bool _play;
    pthread_t pid;
    MUTEX s_mutex;
    uint32_t v_Rtcp_pNum;
    uint32_t v_Rtcp_cNum;
    uint32_t a_Rtcp_pNum;
    uint32_t a_Rtcp_cNum;
    uint32_t ssrc;
    uint8_t pthread_flag;
    void *stream_video_opaque = nullptr;
    void *stream_audio_opaque = nullptr;
    uint64_t frame_timer = 0;//第一帧获取绝对时间

    bool keyFlag = false;

    std::thread audio_thread;
    std::thread video_thread;
    std::thread audio_recv_thread;
    std::thread video_recv_thread;
    bool audio_run = false;
    bool video_run = false;
    bool audio_recv_run = false;
    bool video_recv_run = false;
    List _VideoList;
    List _AudioList;

private:
    
    std::string source_name;
    uint64_t g_audio_clock = 0;
    int channels = 2;
    int bitdep = 2;
    int nb_samples = 1024;
    uint64_t pts_drift;
    MUTEX video_stream_mutex;
    MUTEX audio_stream_mutex;
    
};



#endif