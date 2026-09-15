#ifndef _CLIENTMANAGER_HPP_
#define _CLIENTMANAGER_HPP_

#include <iostream>
#include <list>
#include <map>
// #include "r_media_stream.hpp"
class MEDIA_STREAM;

class SOURCE{
public:
    SOURCE(){
        read_video_callback = NULL;
        read_audio_callback = NULL;
        video_pts = 0;
        audio_pts = 0;
        video_fps = 0;
        audio_sample = 0;
        _play = false;
        pid = 0;
        v_Rtcp_pNum = 0;
        v_Rtcp_cNum = 0;
        a_Rtcp_pNum = 0;
        a_Rtcp_cNum = 0;
        ssrc = 0;
        pthread_flag = 0;
        void *video_opaque = nullptr;
        void *audio_opaque = nullptr;
        bool _run = false;
        media = nullptr;

    }
    ~SOURCE(){};
    typedef int (*read_video)(void *opaque, uint8_t *data, bool keyFrame, uint64_t *pts);
    typedef int (*read_audio)(void *opaque, uint8_t *data, int len, uint64_t *pts);
public:
    std::string video_format;
    std::string audio_format;
    uint64_t video_pts;
    uint64_t audio_pts;
    uint32_t video_fps;
    uint32_t audio_sample;
    read_video read_video_callback;
    read_audio read_audio_callback;
    bool _play;
    pthread_t pid;
    // MUTEX s_mutex;
    uint32_t v_Rtcp_pNum;
    uint32_t v_Rtcp_cNum;
    uint32_t a_Rtcp_pNum;
    uint32_t a_Rtcp_cNum;
    uint32_t ssrc;
    uint8_t pthread_flag;
    void *video_opaque;
    void *audio_opaque;
    MEDIA_STREAM *media;
};

class ClientManager{
public:
    static ClientManager& GetInstance(){
        static ClientManager instance;
        return instance;
    }
    ClientManager(const ClientManager&) = delete;
    ClientManager &operator=(const ClientManager &) = delete;

    int Add_Source(std::string sourceName, int (*read_video_stream)(void *opaque, uint8_t *data, bool keyFrame, uint64_t *pts), void *video_opaque, int (*read_audio_stream)(void *opaque, uint8_t *data, int len, uint64_t *pts), void *audio_opaque, uint32_t videoFrameRate, uint32_t audioSampleRate, std::string videoCodecType, std::string audioCodecType);
    std::map<std::string, SOURCE> rtsp_source;
private:
    ClientManager(){};
    ~ClientManager(){};

};


#endif