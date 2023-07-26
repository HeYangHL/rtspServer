#include "rtspserver.hpp"

//std::map<int, RTSP_SESSION *> RTSP_S::session_map;
std::list<CLI_MSG> RTSP_S::_f_list;
MUTEX RTSP_S::mutex;
MUTEX RTSP_S::source_mutex;
bool RTSP_S::LockFlag=false;
std::map<std::string, SOURCE> RTSP_S::rtsp_session;

/*
*函数描述：添加播放源
*参数：
*	sourceName：rtsp的播放地址
*	read_video_stream:读视频流的回调函数
*	read_audio_stream:读音频流的回调函数
*	videoFrameRate：视频流帧率
*	audioFrameRate：音频流采样率
*	videoCodecType：视频流类型，默认H264
*	audioCodecType：音频流类型，默认aac
*返回值：成功 0
*/
int RTSP_S::add_media_source(std::string sourceName, int (*read_video_stream)(void *opaque, uint8_t *data, int len, uint64_t *pts), int (*read_audio_stream)(void *opaque, uint8_t *data, int len, uint64_t *pts), uint32_t videoFrameRate, uint32_t audioSampleRate, std::string videoCodecType, std::string audioCodecType)
{
	SOURCE new_session;

	new_session.read_video_callback = read_video_stream;
	new_session.read_audio_callback = read_audio_stream;
	new_session.video_fps = videoFrameRate;
	new_session.audio_sample = audioSampleRate;
	new_session.video_format = videoCodecType;
	new_session.audio_format = audioCodecType;

	rtsp_session.insert(std::map<std::string, SOURCE>::value_type(sourceName, new_session));

	return 0;
}

/*
*函数描述：删除数据源
*参数：
*	sourceName：要删除源的路径名
*返回值：成功 0， 失败 -1
*/

int RTSP_S::del_media_source(std::string sourceName)
{
	std::string source_name;
	SOURCE *source_msg;

	std::map<std::string, SOURCE>::iterator source_map;
	std::list<CLI_MSG>::iterator client_msg_list;

    printf("source name : %s\n", sourceName.c_str());
	source_mutex.mutex_lock();
	for(source_map = rtsp_session.begin(); source_map != rtsp_session.end(); source_map++)
	{
		if(source_map->first == sourceName)
		{
            printf("=====>name : %s, pid = %08x\n", sourceName.c_str(), source_map->second.pid);
			source_map->second._play = false;

			pthread_cancel(source_map->second.pid);
			rtsp_session.erase(source_map);
            break;
		}
	}

	source_mutex.mutex_unlock();
#if 1
//	mutex.mutex_lock();

	for(source_map = rtsp_session.begin(); source_map != rtsp_session.end(); source_map++)
                source_map->second.s_mutex.mutex_lock();

	for(client_msg_list = _f_list.begin(); client_msg_list != _f_list.end(); client_msg_list++)
	{
		if(client_msg_list->session_map->get_source_name() == sourceName)
		{
			close(client_msg_list->cli_tcp_fd);
			if(client_msg_list->session_map != NULL)
			{
				delete client_msg_list->session_map;
				client_msg_list->session_map = NULL;
			}
			_f_list.erase(client_msg_list);
			client_msg_list--;
		}
	}
//	mutex.mutex_unlock();
	for(source_map = rtsp_session.begin(); source_map != rtsp_session.end(); source_map++)
                source_map->second.s_mutex.mutex_unlock();
#endif

    return 0;

}


/*
*函数描述：停止rtsp服务器
*返回值：成功 0 失败 -1
*
*/

int RTSP_S::stop()
{
	std::string source_name;
	SOURCE *source_msg = NULL;

	std::list<CLI_MSG>::iterator client_msg_list;
	std::map<std::string, SOURCE>::iterator source_map;

	for(source_map = rtsp_session.begin(); source_map != rtsp_session.end(); source_map++)
                source_map->second.s_mutex.mutex_lock();

	for(client_msg_list = _f_list.begin(); client_msg_list != _f_list.end(); client_msg_list++)
	{
		//关闭套接字
		close(client_msg_list->cli_tcp_fd);		
		//释放会话空间
		if(client_msg_list->session_map)
		{
			delete client_msg_list->session_map;
			client_msg_list->session_map = NULL;
		}
		source_mutex.mutex_lock();
		//关闭线程
		for(source_map = rtsp_session.begin(); source_map != rtsp_session.end(); source_map++)
			pthread_cancel(source_map->second.pid);
		//清除资源列表
		for(source_map = rtsp_session.begin(); source_map != rtsp_session.end(); source_map++)
		{
			rtsp_session.erase(source_map);
			source_map--;
		}
		source_mutex.mutex_unlock();
		//清除会话列表
		_f_list.erase(client_msg_list);
		client_msg_list--;
	}

	for(source_map = rtsp_session.begin(); source_map != rtsp_session.end(); source_map++)
                source_map->second.s_mutex.mutex_unlock();
}


/*
*函数描述：添加客户端到链表
*参数：
*	tcpfd：客户端的tcp套接字
*	cli_ip：客户端ip
*	rtsp_session:客户端对应的会话对象
*返回值：void
*/
void RTSP_S::add_list(int tcpfd, uint8_t *cli_ip, RTSP_SESSION *rtsp_session)
{
	mutex.mutex_lock();
	CLI_MSG cli_msg;

	cli_msg.cli_tcp_fd = tcpfd;
	memcpy(cli_msg.cli_ip, cli_ip, sizeof(cli_msg.cli_ip));
	cli_msg.session_map = rtsp_session;
	cli_msg.server_rtp_fd = serverRtpSockfd;
	cli_msg.server_rtcp_fd = serverRtcpSockfd;

	_f_list.push_back(cli_msg);
	mutex.mutex_unlock();

	return ;
}

/*
*函数描述：创建媒体流发送线程
*参数：void
*返回值：void
*
*/

void RTSP_S::creat_media_pthread(void)
{
	media_stream.start_media();

	return;
}

/*
*函数描述：创建客户端异常检测线程
*参数：void
*返回值：void
*
*/
void RTSP_S::creat_client_list_pthread(void)
{
	cli_list.start_client_listen();
	return;
}

/*
*函数描述：创建tcp套接字
*参数：void
*返回值：void
*
*/
void RTSP_S::creat_sock(void)
{
	open_sock();
	set_sock_opt();
	bind_sock();
	createRtpSocket();
	createRtcpSocket();
	bindRtpSocketAddr();
	bindRtcpSocketAddr();
	createAudioRtpSocket();
	createAudioRtcpSocket();
	bindAudioRtpSocketAddr();
	bindAudioRtcpSocketAddr();
	listen_sock();
//	creat_media_pthread();
//	creat_client_list_pthread();

	return;
}

/*
*函数描述：开始rtsp服务器
*参数：void
*返回值：void
*
*/
void RTSP_S::start(void)
{
	int ret = 0;

	creat_sock();

	return;
}

/*
*函数描述：将创建客户端会话并且将客户端添加到链表
*参数：
*	c_fd：客户端文件描述符
*	cli_ip：客户端ip
*返回值：void
*
*
*/

void RTSP_S::accept_sock(int c_fd, uint8_t *cli_ip)
{
	pthread_t pid_s = 0;
	RTSP_SESSION *rtsp_session = NULL;
	rtsp_session = new RTSP_SESSION;

	printf("start add list!\n");
	add_list(c_fd, cli_ip, rtsp_session);
	printf("end add list!\n");


	rtsp_session->start(c_fd, cli_ip);


	return ;
}
