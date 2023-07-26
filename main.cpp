#include <iostream>
#include <stdio.h>
#include <fstream>
#include "Def.h"
#include "ts_decode.hpp"
#include "rtspserver.hpp"

FILE *v_fout = NULL;
FILE *v_fout_v = NULL;
FILE *a_fout = NULL;
FILE *a_fout_a = NULL;

uint64_t v_pts = 0;
uint64_t a_pts = 0;
uint32_t v_index = 0;
uint32_t a_index = 0;
UDP_TS udp_ts;

int start_Code3(unsigned char *buf)
{
    if (buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
        return 1;
    else
        return 0;
}

int start_Code4(unsigned char *buf)
{
    if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
        return 1;
    else
        return 0;
}

void TsPtsToRtpPts(uint64_t ts_pts, uint64_t *rtp_pts)
{
    *rtp_pts = ts_pts * 9LL / 100LL;

    return;
}

uint8_t *findNext_StartCode(unsigned char *buf, int len)
{
    int i;

    if (len < 3)
        return NULL;

    for (i = 0; i < len - 3; ++i)
    {
        if (start_Code3(buf) || start_Code4(buf))
            return buf;

        ++buf;
    }

    if (start_Code3(buf))
        return buf;

    return NULL;
}

#if 0
int get_video_frame(FILE* file, uint8_t *frame, int size)
{
    int frameSize = 0;

    int rSize = 0;
    unsigned char* nextStartCode = NULL;
    int startCode = 0;

    if(file == NULL)
        return -1;

    rSize = fread((unsigned char *)frame, sizeof(uint8_t), size, file);
#if 0
        printf("====>video data : \n");
        for(int i = 0; i < 64; i++)
        {
            if(i%16 == 0)
                printf("\n");
            printf("%02x ", frame[i]);
        }
        printf("\n");
#endif
    if(!start_Code3(frame) && !start_Code4(frame))
        return -1;
    
    if(start_Code3(frame))
        startCode = 3;
    if(start_Code4(frame))
        startCode = 4;
    
    nextStartCode = findNext_StartCode(frame+startCode, rSize-startCode);
    if(!nextStartCode)
    {
        fseek(file, 0, SEEK_SET);
        frameSize = rSize;
    }
    else
    {
        frameSize = (nextStartCode-frame);
        int ret = fseek(file, (frameSize-rSize), SEEK_CUR);

    }

    return frameSize;
}

int _rv(void * opaque, uint8_t * data, int len, uint64_t *pts)
{
    int ret = 0;

    while(1)
    {
        ret = get_video_frame(v_fout, data, 100000);
#if 0
        printf("====>video data : \n");
        for(int i = 0; i < 64; i++)
        {
            if(i%16 == 0)
                printf("\n");
            printf("%02x ", data[i]);
        }
        printf("\n");
#endif
//        printf("call back frame size = %d\n", ret);
        if(ret > 0)
        {
            if(data[4] == 0x09 || data[4] == 0x6)
            {
                continue;
            }
            v_pts = ((v_index++) * 90000/50);
            *pts = v_pts;
            break;
        }
        
    }
    return ret;
}


int _rvv(void * opaque, uint8_t * data, int len, uint64_t *pts)
{
    int ret = 0;

    while(1)
    {
        ret = get_video_frame(v_fout_v, data, 100000);
#if 0
        printf("====>video data : \n");
        for(int i = 0; i < 64; i++)
        {
            if(i%16 == 0)
                printf("\n");
            printf("%02x ", data[i]);
        }
        printf("\n");
#endif
        if(ret > 0)
        {
            if(data[4] == 0x09 || data[4] == 0x6)
            {
                continue;
            }
            v_pts = ((v_index++) * (90000/25));
            *pts = v_pts;
            break;
        }
        
    }
    return ret;
}
#endif
#if 1
int get_video_frame(uint8_t *frame, int *size, uint64_t *pts)
{

    udp_ts.read_video(frame, size, pts, 10);

    return *size;
}

uint64_t v_first_pts = 0;
uint64_t v_last_pts = 0;

int _rv(void *opaque, uint8_t *data, int len, uint64_t *pts)
{
    int ret = 0;
    int framesize = 0;
    uint64_t ts_pts = 0;
    uint64_t rtp_pts = 0;
    uint64_t pts_nal = 0;

    while (1)
    {
        ret = get_video_frame(data, &framesize, &ts_pts);
        if (ret > 0)
        {
            // TsPtsToRtpPts(ts_pts, &rtp_pts);
            // *pts = rtp_pts;
 //           printf("read video : \n");
 //           print_data(data, 32);
 //           printf("ts video pts = %x\n", ts_pts);
            if (v_first_pts == 0)
            {
                v_first_pts = ts_pts;
                *pts = 0;
            }
            else
            {
                pts_nal = ts_pts - v_first_pts;
                TsPtsToRtpPts(pts_nal, &rtp_pts);
                *pts = rtp_pts;
            }
            break;
        }
    }

    return ret;
}

int get_h264_video_frame(uint8_t *frame, int *size, uint64_t *pts)
{
    udp_ts.read_h264_video(frame, size, pts, 10);

    return *size;
}

uint64_t v_h264_first_pts = 0;
uint64_t v_h264_last_pts = 0;

int _rv1(void *opaque, uint8_t *data, int len, uint64_t *pts)
{
    int ret = 0;
    int framesize = 0;
    uint64_t ts_pts = 0;
    uint64_t rtp_pts = 0;
    uint64_t pts_nal = 0;
    while (1)
    {
        ret = get_h264_video_frame(data, &framesize, &ts_pts);
        if (ret > 0)
        {
            // TsPtsToRtpPts(ts_pts, &rtp_pts);
            // *pts = rtp_pts;
//            printf("read H264 video frame :\n");
//            print_data(data, 32);
//            printf("ts H264 pts = %x\n", ts_pts);
            #if 1
            if (v_h264_first_pts == 0)
            {
                v_h264_first_pts = ts_pts;
                *pts = 0;
            }
            else
            {
                pts_nal = ts_pts - v_h264_first_pts;
                TsPtsToRtpPts(pts_nal, &rtp_pts);
                *pts = rtp_pts;
            }
            #endif
            break;
        }
    }

    return ret;
}
#endif
#if 0
int get_aac_frame_len(FILE *file)
{
    int r_len = 0;
    uint8_t adts[7] = {0};
    short aac_len = 0;
    
    r_len = fread(adts, sizeof(uint8_t), sizeof(adts), file);
    if(r_len <= 0)
        fseek(file, 0, SEEK_SET);

//    printf("%02x %02x %02x\n", adts[3], adts[4], adts[5]);
//    printf("===>%02x\n", ((adts[5]&0xe0) >> 5));

    aac_len |= ((adts[3]&0x3) << 11);
    aac_len |= (adts[4] << 3);
    aac_len |= ((adts[5]&0xe0) >> 5);

    fseek(file, -7, SEEK_CUR);

    return aac_len;
}

int get_aac_frame(FILE *file, uint8_t *frame)
{
    int frameSize = 0;
    int r_len = 0;

    frameSize = get_aac_frame_len(file);
    r_len = fread(frame, sizeof(uint8_t), frameSize, file);

    return r_len;
}

int _ra(void * opaque, uint8_t * data, int len, uint64_t *pts)
{
    int ret = 0;

    while(1)
    {
        ret = get_aac_frame(a_fout, data);
        if(ret > 0)
        {
           a_pts += 1024*(90000/48000);
            *pts = a_pts;
            break;
        }
    }
    return ret;
}
int _raa(void * opaque, uint8_t * data, int len, uint64_t *pts)
{
    int ret = 0;

    while(1)
    {
        ret = get_aac_frame(a_fout_a, data);
        if(ret > 0)
        {
//            a_pts = ((a_index++) * (1024*(90000/48000)));
            a_pts += 1024*(90000/48000);
            *pts = a_pts;
            break;
        }
    }
    return ret;
}
#endif
#if 1
int get_aac_frame(uint8_t *data, int *framesize, uint64_t *pts)
{
    udp_ts.read_audio(data, framesize, pts, 10);

    return *framesize;
}

uint64_t a_first_pts = 0;
uint64_t a_last_pts = 0;

int _ra(void *opaque, uint8_t *data, int len, uint64_t *pts)
{
    int ret = 0;
    int framesize = 0;
    uint64_t ts_pts = 0;
    uint64_t rtp_pts = 0;
    uint64_t pts_nal = 0;

    while (1)
    {
        ret = get_aac_frame(data, &framesize, &ts_pts);
        if (ret > 0)
        {
            #if 1
            if (a_first_pts == 0)
            {
                a_first_pts = ts_pts;
                *pts = 0;
            }
            else
            {
                pts_nal = ts_pts - a_first_pts;
                TsPtsToRtpPts(pts_nal, &rtp_pts);
                *pts = rtp_pts;
            }
            #endif
            // TsPtsToRtpPts(ts_pts, &rtp_pts);
            // *pts = rtp_pts;

            break;
        }
    }
    return ret;
}

int get_aac_h264_frame(uint8_t *data, int *framesize, uint64_t *pts)
{
    udp_ts.read_h264_audio(data, framesize, pts, 10);

    return *framesize;
}
uint64_t a_h264_first_pts = 0;
uint64_t a_h264_last_pts = 0;

int _ra1(void *opaque, uint8_t *data, int len, uint64_t *pts)
{
    int ret = 0;
    int framesize = 0;
    uint64_t ts_pts = 0;
    uint64_t rtp_pts = 0;
    uint64_t pts_nal = 0;

    while (1)
    {
        ret = get_aac_h264_frame(data, &framesize, &ts_pts);
        if (ret > 0)
        {
            #if 1
            if (a_h264_first_pts == 0)
            {
                a_h264_first_pts = ts_pts;
                *pts = 0;
            }
            else
            {
                pts_nal = ts_pts - a_h264_first_pts;
                TsPtsToRtpPts(pts_nal, &rtp_pts);
                *pts = rtp_pts;
            }
            #endif
            // TsPtsToRtpPts(ts_pts, &rtp_pts);
            // *pts = rtp_pts;

            break;
        }
    }
    return ret;
}
#endif
int main(void)
{
#if 0
    RTSP_S rtsp_s;
    
    v_fout = fopen("video.h264", "rb");
    if(v_fout == NULL)
    {
        perror("open file error");
        return -1;
    }
    a_fout = fopen("audio.aac", "rb");
    if(a_fout == NULL)
    {
        perror("open file error");
        return -1;
    }
    v_fout_v = fopen("test.h264", "rb");
    if(v_fout_v == NULL)
    {
        perror("open file error");
        return -1;
    }
    a_fout_a = fopen("audio_2.aac", "rb");
    if(a_fout_a == NULL)
    {
        perror("open file error");
        return -1;
    }
#endif

    RTSP_S rtsp_s;

    rtsp_s.add_media_source("test_media", _rv, _ra, 60, 48000, "h265", "aac");
    
    rtsp_s.start();
    udp_ts.start_decode();
    rtsp_s.add_media_source("test_video", _rv1, _ra1, 60, 48000, "h264", "aac");

 //   sleep(10);
 //   rtsp_s.del_media_source("test_video");
    getchar();

    rtsp_s.stop();


    return 0;
}
