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

#if 1
int get_video_frame(FILE *file, uint8_t *frame, int size)
{
    int frameSize = 0;

    int rSize = 0;
    unsigned char *nextStartCode = NULL;
    int startCode = 0;

    if (file == NULL)
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
    if (!start_Code3(frame) && !start_Code4(frame))
        return -1;

    if (start_Code3(frame))
        startCode = 3;
    if (start_Code4(frame))
        startCode = 4;

    nextStartCode = findNext_StartCode(frame + startCode, rSize - startCode);
    if (!nextStartCode)
    {

        fseek(file, 0, SEEK_SET);
        frameSize = rSize;
    }
    else
    {
        frameSize = (nextStartCode - frame);
        int ret = fseek(file, (frameSize - rSize), SEEK_CUR);
    }

    return frameSize;
}

// 获取视频时间戳
uint64_t calculate_video_timestamp()
{
    // 正确的时间戳增量：1024 * 90000 / 48000 = 1920
    static uint32_t frame_count = 0;
    static const uint64_t timestamp_increment = 90000ULL / 30;
    frame_count++;
    return frame_count * timestamp_increment;
}

// 高精度睡眠函数
void precise_usleep(long microseconds)
{
    struct timespec req = {
        .tv_sec = microseconds / 1000000,
        .tv_nsec = (microseconds % 1000000) * 1000};
    struct timespec rem;

    while (nanosleep(&req, &rem) == -1)
    {
        req = rem;
    }
}

static struct timespec global_start_time;
static int global_initialized = 0;

void global_init_timing()
{
    clock_gettime(CLOCK_MONOTONIC, &global_start_time);
    global_initialized = 1;
}

void sync_to_global_timestamp(uint64_t expected_pts)
{
    if (!global_initialized)
    {
        global_init_timing();
        return;
    }

    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    double elapsed = (current_time.tv_sec - global_start_time.tv_sec) +
                     (current_time.tv_nsec - global_start_time.tv_nsec) / 1e9;

    double expected_time = (double)expected_pts / 90000.0;

    double sleep_time = expected_time - elapsed;

    if (sleep_time > 0)
    {
        precise_usleep(sleep_time * 1000000);
    }
}


int _rv(void *opaque, uint8_t *data, bool keyFrame, uint64_t *pts)
{
    int ret = 0;
    static int video_frame_count = 0;
    static uint64_t video_first_pts = 0;
    static int video_first_frame = 1;

    while (1)
    {
        ret = get_video_frame(v_fout, data, 1000000);
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
        //    printf("call back frame size = %d\n", ret);
        if (ret > 0)
        {
            if (data[4] == 0x09 || data[4] == 0x6)
            {
                continue;
            }
            // v_pts = ((v_index++) * 90000LL / 30);
            // *pts = v_pts;
            // usleep(33333);
            *pts = calculate_video_timestamp();
            // printf("========>video v_pts : %llu\n", v_pts);
            // 记录第一帧的时间戳
            if (video_first_frame)
            {
                video_first_pts = *pts;
                video_first_frame = 0;
            }

            // 同步到正确的时间
            sync_to_global_timestamp(*pts - video_first_pts);

            video_frame_count++;
            if (video_frame_count % 100 == 0)
            {
                printf("Video frame %d: size=%d, PTS=%llu\n", video_frame_count, ret, *pts);
            }
            break;

        }
    }
    return ret;
}

int _rvv(void *opaque, uint8_t *data, int len, uint64_t *pts)
{
    int ret = 0;

    while (1)
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
        if (ret > 0)
        {
            if (data[4] == 0x09 || data[4] == 0x6)
            {
                continue;
            }
            v_pts = ((v_index++) * (90000 / 24));
            *pts = v_pts;
            break;
        }
    }
    return ret;
}
#endif
#if 0
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
#if 1
int get_aac_frame_len(FILE *file)
{
    int r_len = 0;
    uint8_t adts[7] = {0};
    short aac_len = 0;

    r_len = fread(adts, sizeof(uint8_t), sizeof(adts), file);
    if (r_len <= 0)
        fseek(file, 0, SEEK_SET);

    //    printf("%02x %02x %02x\n", adts[3], adts[4], adts[5]);
    //    printf("===>%02x\n", ((adts[5]&0xe0) >> 5));

    aac_len |= ((adts[3] & 0x3) << 11);
    aac_len |= (adts[4] << 3);
    aac_len |= ((adts[5] & 0xe0) >> 5);

    fseek(file, -7, SEEK_CUR);

    return aac_len;
}
#if 0
// 检查是否为有效的ADTS头
bool is_valid_adts(const uint8_t *adts) {
    // 检查同步字：前12位必须是0xFFF
    if (adts[0] != 0xFF || (adts[1] & 0xF0) != 0xF0) {
        return false;
    }
    // 其他可选检查（如层、配置等）可根据需要添加
    return true;
}

int get_aac_frame_len(FILE *file) {
    uint8_t adts[7];
    int r_len;
    int attempts = 0;
    const int max_attempts = 1024; // 避免无限循环，最多尝试1024次

    while (attempts < max_attempts) {
        r_len = fread(adts, sizeof(uint8_t), sizeof(adts), file);
        if (r_len < sizeof(adts)) {
            // 文件结束或读取错误
            fseek(file, 0, SEEK_SET);
            return -1;
        }

        if (is_valid_adts(adts)) {
            // 解析ADTS帧长度（第3到第5字节的部分位）
            int aac_len = 0;
            aac_len |= ((adts[3] & 0x03) << 11); // 取第3字节的低2位，放到高位
            aac_len |= (adts[4] << 3);           // 第4字节的8位，放到中间
            aac_len |= ((adts[5] & 0xE0) >> 5);  // 取第5字节的高3位，放到低位

            // 回溯文件指针到ADTS头开始处
            fseek(file, -7, SEEK_CUR);
            return aac_len;
        } else {
            // 不是有效的ADTS头，向后移动1字节继续搜索
            fseek(file, -6, SEEK_CUR); // 因为读了7字节，现在回退6字节（即前进1字节）
            attempts++;
        }
    }

    // 尝试多次后未找到有效的ADTS头
    fseek(file, 0, SEEK_SET);
    return -1;
}
#endif
int get_aac_frame(FILE *file, uint8_t *frame)
{
    int frameSize = 0;
    int r_len = 0;

    frameSize = get_aac_frame_len(file);
    r_len = fread(frame, sizeof(uint8_t), frameSize, file);

    return r_len;
}
// 获取音频时间戳
uint64_t calculate_audio_timestamp()
{
    // 正确的时间戳增量：1024 * 90000 / 48000 = 1920
    static uint32_t frame_count = 0;
    static const uint64_t timestamp_increment = 1024 * 90000ULL / 48000;
    frame_count++;
    return frame_count * timestamp_increment;
}

int _ra(void *opaque, uint8_t *data, int len, uint64_t *pts)
{
    int ret = 0;
    static int frame_count = 0;
    static uint64_t first_pts = 0;
    static int first_frame = 1;

    while (1)
    {
        ret = get_aac_frame(a_fout, data);
        // printf("========>audio ret : %d\n", ret);
        if (ret > 0)
        {
            // a_pts = (a_index++)*1024*90000LL/48000;
            // a_pts += 1024 * 90000ULL / 48000;
            *pts = calculate_audio_timestamp();
            // printf("========>audio a_pts : %llu\n", *pts);
            // struct timespec req = {0, 21333 * 1000}; // 21.333ms
            // nanosleep(&req, NULL);

            // 记录第一帧的时间戳
            if (first_frame)
            {
                first_pts = *pts;
                first_frame = 0;
            }

            // 同步到正确的时间
            sync_to_global_timestamp(*pts - first_pts);

            frame_count++;
            if (frame_count % 100 == 0)
            {
                printf("Audio frame %d: size=%d, PTS=%llu\n", frame_count, ret, *pts);
            }

            break;
        }
    }

    return ret;
}
int _raa(void *opaque, uint8_t *data, int len, uint64_t *pts)
{
    int ret = 0;

    while (1)
    {
        ret = get_aac_frame(a_fout_a, data);
        if (ret > 0)
        {
            //            a_pts = ((a_index++) * (1024*(90000/48000)));
            a_pts += 1024 * (90000 / 48000);
            *pts = a_pts;
            break;
        }
    }
    return ret;
}
#endif
#if 0
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
#if 1

    v_fout = fopen("bbb_sunflower_1080p_30fps_normal.h264", "rb");
    if (v_fout == NULL)
    {
        perror("open file error");
        return -1;
    }
    a_fout = fopen("bbb_sunflower_1080p_30fps_normal.aac", "rb");
    if (a_fout == NULL)
    {
        perror("open file error");
        return -1;
    }
    // v_fout_v = fopen("Sintel.2010.1080p.h264", "rb");
    // if (v_fout_v == NULL)
    // {
    //     perror("open file error");
    //     return -1;
    // }
    // a_fout_a = fopen("Sintel.2010.1080p.aac", "rb");
    // if (a_fout_a == NULL)
    // {
    //     perror("open file error");
    //     return -1;
    // }
#endif

    RTSP_S rtsp_s;
    global_init_timing(); // 提前初始化全局时钟
    rtsp_s.add_media_source("test_media", _rv, NULL, NULL, NULL, 30, 48000, "h264", "aac");

    rtsp_s.start();
    udp_ts.start_decode();
    //   rtsp_s.add_media_source("test_video", _rvv, _raa, 60, 48000, "h264", "aac");

    //   sleep(10);
    //   rtsp_s.del_media_source("test_video");
    getchar();

    rtsp_s.stop();

    return 0;
}
