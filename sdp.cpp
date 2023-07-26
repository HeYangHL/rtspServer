#include "sdp.hpp"
#include "rtspserver.hpp"

/*
*函数描述：初始化DESCRIBE信令回复的sdp媒体描述信息
*参数：
*	content: 会话的路径名
*	cli_fd：客户端文件描述符
*返回值：
*	成功 0 失败 -1
*/

int SDP_M::init_sdp(const char *content, int cli_fd)
{
	RTSP_S rtsp_s;
	char video_media[1024] = {0};
	char audio_media[1024] = {0};
	int flag = 0;
	char video[1024] = {0};
	char audio[1024] = {0};
	std::string source_name;
	SOURCE *source_msg = NULL;


	rtsp_source_name = content;

	std::map<std::string, SOURCE>::iterator source_map;

	for(source_map = rtsp_s.rtsp_session.begin(); source_map != rtsp_s.rtsp_session.end(); source_map++)
	{
		if(source_map->first == content)
		{
			memset(m_sdp, 0, sizeof(m_sdp));
			snprintf(m_sdp, sizeof(m_sdp),
					"v=2\r\n"
					"o=- 91675836373 1 IN IP4 0.0.0.0\r\n"
					"t=0 0\r\n"
					"a=control:*\r\n"
					"a=type:broadcast\r\n"
					"m=video 0 RTP/AVP 96\r\n"
					"c=IN IP4 0.0.0.0\r\n"
					"a=rtpmap:96 %s/90000\r\n"
					"a=framerate:%d\r\n"
					"a=control:track0\r\n"
					"m=audio 0 RTP/AVP 97\r\n"
					"c=IN IP4 0.0.0.0\r\n"
					"a=rtpmap:97 mpeg4-generic/%d/2\r\n"
					"a=fmtp:97 profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3;config=1190\r\n"
					"a=control:track1\r\n",
					source_map->second.video_format.c_str(), source_map->second.video_fps, source_map->second.audio_sample
					);
		}
	}
	return 0;
}

