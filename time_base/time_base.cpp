#include "time_base.hpp"

/***
 * 视频时间基处理
 * 
 */
timebase_type_t TimeBase::video_detect_timebase(uint64_t current_pts, uint64_t prev_pts, int fps) {
    if (prev_pts == 0) {
        return TIMEBASE_UNKNOWN;
    }
    
    int64_t diff = current_pts - prev_pts;
    if (diff <= 0) {
        return TIMEBASE_UNKNOWN;
    }
    int timebase = diff * fps;
    if(990000 < timebase && timebase < 1005000)
        return TIMEBASE_MICROSEC;
    else if(999990000 < timebase && timebase < 1000050000)
        return TIMEBASE_NANOSEC;
    else if(990 < timebase && timebase < 1010)
        return TIMEBASE_MILLISEC;
    else if(89900 < timebase && timebase < 90500)
        return TIMEBASE_90KHZ;
    else
        return TIMEBASE_UNKNOWN;

#if 0
    // 常见帧率对应的间隔
    const struct {
        int64_t interval;
        timebase_type_t timebase;
        const char *name;
    } timebase_candidates[] = {
        {33,     TIMEBASE_90KHZ,    "90kHz"},      // 30fps: 3000/90 = 33
        {66,     TIMEBASE_90KHZ,    "90kHz"},      // 15fps: 6000/90 = 66
        {16,     TIMEBASE_90KHZ,    "90kHz"},      // 60fps: 1500/90 = 16
        {33333,  TIMEBASE_MICROSEC, "microsec"},   // 30fps: 33ms = 33333us
        {16666,  TIMEBASE_MICROSEC, "microsec"},   // 60fps: 16ms = 16666us
        {40000,  TIMEBASE_MICROSEC, "microsec"},   // 25fps: 40ms = 40000us
        {33,     TIMEBASE_MILLISEC, "millisec"},   // 30fps: 33ms
        {16,     TIMEBASE_MILLISEC, "millisec"},   // 60fps: 16ms
        {0,      TIMEBASE_UNKNOWN,  "unknown"}
    };
    
    // 检查是否匹配候选时间基
    for (int i = 0; timebase_candidates[i].interval > 0; i++) {
        if (llabs(diff - timebase_candidates[i].interval) <= 1) {
            printf("Detected timebase: %s (frame interval: %ld)\n", 
                   timebase_candidates[i].name, diff);
            return timebase_candidates[i].timebase;
        }
    }
    
    // 如果都不匹配，可能是其他时钟频率
    if (diff > 1000 && diff < 1000000) {
        // 假设是其他视频时钟频率
        converter.clock_rate = (uint32_t)(1000000.0 / (diff / 1000.0)); // 估算时钟频率
        printf("Detected custom clock rate: %u Hz (frame interval: %ld)\n", 
               converter.clock_rate, diff);
        return TIMEBASE_CLOCK_RATE;
    }
#endif
    return TIMEBASE_UNKNOWN;
}

uint64_t TimeBase::video_convert_to_90khz(uint64_t pts, timebase_type_t timebase, uint32_t clock_rate) {
    switch (timebase) {
        case TIMEBASE_90KHZ:
            // 已经是90kHz，直接返回
            return (uint32_t)pts;
            
        case TIMEBASE_MILLISEC:
            // 毫秒 -> 90kHz: ms * 90
            return (uint32_t)(pts * 90);
            
        case TIMEBASE_MICROSEC:
            // 微秒 -> 90kHz: us * 90 / 1000
            return (uint32_t)(pts * 90 / 1000);
            
        case TIMEBASE_NANOSEC:
            // 纳秒 -> 90kHz: ns * 90 / 1000000
            return (uint32_t)(pts * 90 / 1000000);
            
        case TIMEBASE_CLOCK_RATE:
            // 其他时钟频率 -> 90kHz: pts * 90000 / clock_rate
            if (clock_rate > 0) {
                return (uint32_t)(pts * 90000ULL / clock_rate);
            }
            // 降级到微秒处理
            return (uint32_t)(pts * 90 / 1000);
            
        case TIMEBASE_UNKNOWN:
        default:
            // 未知时间基，假设为微秒
            printf("Warning: Unknown timebase, assuming microseconds\n");
            return (uint32_t)(pts * 90 / 1000);
    }
}

uint32_t TimeBase::video_get_source_timebase(uint64_t input_pts, int fps)
{
    if (!converter.initialized) {
        // 第一帧初始化
        converter.base_pts = input_pts;
        converter.base_rtp_ts = 0;
        converter.last_rtp_ts = 0;
        converter.last_input_pts = input_pts;
        converter.initialized = 1;
        converter.frame_count = 1;
        return 0;
    }
    
    // 检测时间基（需要至少2帧）
    if (converter.timebase == TIMEBASE_UNKNOWN && converter.frame_count >= 1) {
        converter.timebase = video_detect_timebase(input_pts, converter.last_input_pts, fps);
        
        if (converter.timebase == TIMEBASE_UNKNOWN) {
            // 如果无法检测，假设为微秒（海思VENC常见）
            converter.timebase = TIMEBASE_MICROSEC;
            printf("Timebase detection failed, assuming microseconds\n");
        }
    }

    return 0;
}

// 主接口：转换时间戳到90kHz RTP时间戳
uint64_t TimeBase::video_convert_pts_to_rtp_timestamp(uint64_t input_pts) {
    
    while(TIMEBASE_UNKNOWN == converter.timebase);
    // 转换为90kHz
    uint64_t rtp_timestamp = video_convert_to_90khz(input_pts - converter.base_pts, 
                                            converter.timebase, 
                                            converter.clock_rate);
    
    // // 确保时间戳单调递增
    // if (rtp_timestamp <= converter.last_rtp_ts) {
    //     // 时间戳回退，使用上一时间戳+增量
    //     uint32_t expected_increment = 3000; // 30fps的增量：90000/30 = 3000
    //     rtp_timestamp = converter.last_rtp_ts + expected_increment;
    //     printf("Warning: RTP timestamp rollback detected, using incremental value\n");
    // }
    
    // converter.last_rtp_ts = rtp_timestamp;
    converter.last_input_pts = input_pts;
    converter.frame_count++;
    
    return rtp_timestamp;
}

// 获取检测到的时间基信息
const char* TimeBase::video_get_timebase_info(void) {
    static char info[64];
    
    switch (converter.timebase) {
        case TIMEBASE_90KHZ:
            return "90kHz (no conversion needed)";
        case TIMEBASE_MILLISEC:
            return "milliseconds";
        case TIMEBASE_MICROSEC:
            return "microseconds";
        case TIMEBASE_NANOSEC:
            return "nanoseconds";
        case TIMEBASE_CLOCK_RATE:
            snprintf(info, sizeof(info), "custom clock rate: %u Hz", converter.clock_rate);
            return info;
        case TIMEBASE_UNKNOWN:
        default:
            return "unknown";
    }
}

/***
 * 音频时间基处理
 * 
 */

int TimeBase::get_typical_samples_per_frame(std::string codec_name) {
    if (codec_name.empty()) return 1024; // 默认值
    
    if (codec_name.compare("aac") == 0) return 1024;
    if (codec_name.compare("mp3") == 0) return 1152;
    if (codec_name.compare("ac3") == 0) return 1536;
    if (codec_name.compare("opus") == 0) return 960;  // 常见值
    
    return 1024; // 默认值
}

timebase_type_t TimeBase::audio_detect_timebase(uint64_t current_pts, uint64_t prev_pts, int sample_rate, std::string type) {
    if (prev_pts == 0) {
        return TIMEBASE_UNKNOWN;
    }
    
    int samples_per_frame = get_typical_samples_per_frame(type);

    int64_t diff = current_pts - prev_pts;
    if (diff <= 0) {
        return TIMEBASE_UNKNOWN;
    }
    int timebase = (diff * sample_rate) / samples_per_frame;
    if(990000 < timebase && timebase < 1005000)
        return TIMEBASE_MICROSEC;
    else if(999990000 < timebase && timebase < 1000050000)
        return TIMEBASE_NANOSEC;
    else if(990 < timebase && timebase < 1010)
        return TIMEBASE_MILLISEC;
    else if(89900 < timebase && timebase < 90500)
        return TIMEBASE_90KHZ;
    else
        return TIMEBASE_UNKNOWN;


    return TIMEBASE_UNKNOWN;
}

uint64_t TimeBase::audio_convert_to_90khz(uint64_t pts, timebase_type_t timebase, uint32_t clock_rate) {
    switch (timebase) {
        case TIMEBASE_90KHZ:
            // 已经是90kHz，直接返回
            return (uint32_t)pts;
            
        case TIMEBASE_MILLISEC:
            // 毫秒 -> 90kHz: ms * 90
            return (uint32_t)(pts * 90);
            
        case TIMEBASE_MICROSEC:
            // 微秒 -> 90kHz: us * 90 / 1000
            return (uint32_t)(pts * 90 / 1000);
            
        case TIMEBASE_NANOSEC:
            // 纳秒 -> 90kHz: ns * 90 / 1000000
            return (uint32_t)(pts * 90 / 1000000);
            
        case TIMEBASE_CLOCK_RATE:
            // 其他时钟频率 -> 90kHz: pts * 90000 / clock_rate
            if (clock_rate > 0) {
                return (uint32_t)(pts * 90000ULL / clock_rate);
            }
            // 降级到微秒处理
            return (uint32_t)(pts * 90 / 1000);
            
        case TIMEBASE_UNKNOWN:
        default:
            // 未知时间基，假设为微秒
            printf("Warning: Unknown timebase, assuming microseconds\n");
            return (uint32_t)(pts * 90 / 1000);
    }
}

uint32_t TimeBase::audio_get_source_timebase(uint64_t input_pts, int sample_rate, std::string type)
{
    if (!audio_converter.initialized) {
        // 第一帧初始化
        audio_converter.base_pts = input_pts;
        audio_converter.base_rtp_ts = 0;
        audio_converter.last_rtp_ts = 0;
        audio_converter.last_input_pts = input_pts;
        audio_converter.initialized = 1;
        audio_converter.frame_count = 1;
        return 0;
    }
    
    // 检测时间基（需要至少2帧）
    if (audio_converter.timebase == TIMEBASE_UNKNOWN && audio_converter.frame_count >= 1) {
        audio_converter.timebase = audio_detect_timebase(input_pts, audio_converter.last_input_pts, sample_rate, type);
        
        if (audio_converter.timebase == TIMEBASE_UNKNOWN) {
            // 如果无法检测，假设为微秒（海思VENC常见）
            audio_converter.timebase = TIMEBASE_MICROSEC;
            printf("Timebase detection failed, assuming microseconds\n");
        }
    }

    return 0;
}

// 主接口：转换时间戳到90kHz RTP时间戳
uint64_t TimeBase::audio_convert_pts_to_rtp_timestamp(uint64_t input_pts) {
    
    while(TIMEBASE_UNKNOWN == audio_converter.timebase);
    // 转换为90kHz
    uint64_t rtp_timestamp = audio_convert_to_90khz(input_pts - audio_converter.base_pts, 
                                            audio_converter.timebase, 
                                            audio_converter.clock_rate);
    
    // 确保时间戳单调递增
    if (rtp_timestamp <= audio_converter.last_rtp_ts) {
        // 时间戳回退，使用上一时间戳+增量
        uint32_t expected_increment = 3000; // 30fps的增量：90000/30 = 3000
        rtp_timestamp = audio_converter.last_rtp_ts + expected_increment;
        printf("Warning: RTP timestamp rollback detected, using incremental value\n");
    }
    
    audio_converter.last_rtp_ts = rtp_timestamp;
    audio_converter.last_input_pts = input_pts;
    audio_converter.frame_count++;
    
    return rtp_timestamp;
}

// 获取检测到的时间基信息
const char* TimeBase::audio_get_timebase_info(void) {
    static char info[64];
    
    switch (audio_converter.timebase) {
        case TIMEBASE_90KHZ:
            return "90kHz (no conversion needed)";
        case TIMEBASE_MILLISEC:
            return "milliseconds";
        case TIMEBASE_MICROSEC:
            return "microseconds";
        case TIMEBASE_NANOSEC:
            return "nanoseconds";
        case TIMEBASE_CLOCK_RATE:
            snprintf(info, sizeof(info), "custom clock rate: %u Hz", audio_converter.clock_rate);
            return info;
        case TIMEBASE_UNKNOWN:
        default:
            return "unknown";
    }
}