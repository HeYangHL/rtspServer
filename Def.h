#ifndef _DEF_H_
#define _DEF_H_

#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <endian.h>



typedef enum{
	RTSP_OPTIONS = 0,
	RTSP_DESCRIBE = 1,
	RTSP_SETUP = 2,
	RTSP_PLAY = 3,
	RTSP_PAUSE = 4,
	RTSP_TEARDOWN = 5,
	RTSP_SET_PARAMETER = 6,
	RTSP_GET_PARAMETER = 7,
	RTSP_METHOD_MAX
}RtspMethodT;

typedef enum{
	RTSP_VIDEO_STREAM = 0,
	RTSP_AUDIO_STREAM
}RtpStream;


struct rtsp_method{
    int method;
    const char *method_str;
};

extern const rtsp_method s_method[RTSP_METHOD_MAX];

struct rtsp_code{
    int code;
    const char *code_str;
};

#define TS_PKT_SIZE 188
#if 0
typedef struct TS_Header{
    unsigned sync_byte                      :8;//同步字节，固定为0x47
    unsigned transport_error_indicator      :1;//错误指示符
    unsigned payload_unit_start_indicator   :1;//负载单元起始标识符
    unsigned transport_priority             :1;//传输优先级，0低优先级 1高优先级
    unsigned pid                            :13;//pid值
    unsigned transport_scrambling_cntrol    :2;//传输加扰控制 00：未加密
    unsigned adaptation_field_control       :2;//是否包含自适应区
    unsigned continuity_counter             :4;//递增计数器
    unsigned adjust_byte                    :8;
}TS_Header;
#endif
//rtp基于tcp传输的包头
typedef struct RtpTcpHdr
{
	int8_t	dollar;
	int8_t	channel;
	int16_t	len;
}RtpTcpHdr;

//rtp header
typedef struct RtpHdr
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	uint8_t cc : 4,  			// CSRC count
			x : 1,   			// header extend
			p : 1,   			// padding flag
			version : 2;		// version
	uint8_t pt : 7,				// payload type
			m : 1;				// mark bit
#elif __BYTE_ORDER == __BIG_ENDIAN
#error "unsupported big-endian"
#else
#error "Please fix <endian.h>"
#endif
	uint16_t seq;				// sequence number;
	uint32_t ts;				// timestamp
	uint32_t ssrc;				// sync source
}RtpHdr;

typedef struct RtcpHdr
{
	uint8_t rc :5,
			p :1,
			v :2;
	uint8_t pt;
	uint16_t lenght;
	uint32_t ssrc;
	uint32_t ntp_sec;
	uint32_t ntp_usec;
	uint32_t pts;
	uint32_t p_count;
	uint32_t c_count;	
}RtcpHdr;

typedef struct H265RtpLoad{
	uint16_t tid :3,
			 layerid :6,
			 type :6,
			 f :5;
	


}H264RtpLoad;

#define SERVERRTPPORT 55532
#define SERVERRTCPPORT 55533
#define AUDIO_S_RTPPORT 68400
#define AUDIO_S_RTCPPORT 68401


extern const rtsp_code s_code[15];

#define TS_MEDIA 0
#define H264_MEDIA 1


#define ERROR_NO errno
#define ERROR_STR strerror( errno )
#define N_EINPROGRESS EINPROGRESS
#define N_C_EWOULDBLOCK EINPROGRESS
#define N_EINTR EINTR
#define N_EINPROGRESS EINPROGRESS
#define N_EWOULDBLOCK EWOULDBLOCK
#define N_EAGAIN EWOULDBLOCK

#define MAX_RECV_BUF_LEN 1024*4

void print_data(unsigned char *buf, int len);

#endif
