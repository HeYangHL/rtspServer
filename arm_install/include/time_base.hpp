#ifndef _TIME_BASE_HPP_
#define _TIME_BASE_HPP_
/***************************************************************************************
 * time_base.cpp,time_base.hpp功能描述：
 * 文件主要是为了检测媒体源时间戳的时间基是不是90khz；因为rtsp的时间戳需要时间基为90khz
 * 如果媒体源的时间戳的时间基不是90khz需要将时间戳转化为基于90khz的时间戳。
 ***************************************************************************************/
#include <iostream>
#include <stdio.h>
#include <string.h>

typedef enum
{
    TIMEBASE_UNKNOWN = 0,
    TIMEBASE_90KHZ,     // 90,000 Hz
    TIMEBASE_MILLISEC,  // 1,000 Hz
    TIMEBASE_MICROSEC,  // 1,000,000 Hz
    TIMEBASE_NANOSEC,   // 1,000,000,000 Hz
    TIMEBASE_CLOCK_RATE // 其他时钟频率
} timebase_type_t;

typedef struct
{
    timebase_type_t timebase;
    uint32_t clock_rate;     // 如果是TIMEBASE_CLOCK_RATE，存储实际频率
    uint64_t base_pts;       // 基准PTS
    uint32_t base_rtp_ts;    // 基准RTP时间戳
    uint32_t last_rtp_ts;    // 最后RTP时间戳
    uint64_t last_input_pts; // 最后输入PTS
    int frame_count;         // 帧计数
    int initialized;         // 初始化标志
} timebase_converter_t;

class TimeBase
{
public:
    TimeBase()
    {
        converter.timebase = TIMEBASE_UNKNOWN;
        converter.clock_rate = 90000;
        converter.base_pts = 0;
        converter.base_rtp_ts = 0;
        converter.last_rtp_ts = 0;
        converter.last_input_pts = 0;
        converter.frame_count = 0;
        converter.initialized = 0;
    };
    ~TimeBase() {};
    timebase_type_t video_detect_timebase(uint64_t current_pts, uint64_t prev_pts, int fps);
    uint64_t video_convert_to_90khz(uint64_t pts, timebase_type_t timebase, uint32_t clock_rate);
    uint64_t video_convert_pts_to_rtp_timestamp(uint64_t input_pts);
    uint32_t video_get_source_timebase(uint64_t input_pts, int fps);
    const char *video_get_timebase_info(void);

    timebase_type_t audio_detect_timebase(uint64_t current_pts, uint64_t prev_pts, int sample_rate, std::string type);
    uint64_t audio_convert_to_90khz(uint64_t pts, timebase_type_t timebase, uint32_t clock_rate);
    uint64_t audio_convert_pts_to_rtp_timestamp(uint64_t input_pts);
    uint32_t audio_get_source_timebase(uint64_t input_pts, int sample_rate, std::string type);
    int get_typical_samples_per_frame(std::string codec_name);
    const char *audio_get_timebase_info(void);

private:
    timebase_converter_t converter;
    timebase_converter_t audio_converter;
};

#endif