#include "clientsession.hpp"
#include "rtspserver.hpp"

bool RTSP_SESSION::_play = false;

/*
 *函数描述：清除已经断开连接客户端资源
 *参数：
 *   fd: 断开连接客户端的文件描述符
 *返回值：
 *   成功 0 失败 -1
 */

int RTSP_SESSION::clear_source(int fd)
{
    RTSP_S rtsp_s;

    std::list<CLI_MSG>::iterator client_msg_list;
    std::list<CLI_MSG>::iterator client_is_empty;
    std::map<std::string, SOURCE>::iterator source_msg;

    for (client_msg_list = rtsp_s._f_list.begin(); client_msg_list != rtsp_s._f_list.end(); client_msg_list++)
    {
        //         printf("client num = %d, iter fd = %d, fd = %d\n", rtsp_s._f_list.size(), iter->cli_tcp_fd, fd);
        if (client_msg_list->cli_tcp_fd == fd)
        {

            close(fd);
            // 给所有的资源加锁
            for (source_msg = rtsp_s.rtsp_session.begin(); source_msg != rtsp_s.rtsp_session.end(); source_msg++)
                source_msg->second.s_mutex.mutex_lock();

            //           rtsp_s.mutex.mutex_lock();
            if (client_msg_list->session_map != NULL)
            {
                delete client_msg_list->session_map;
                client_msg_list->session_map = NULL;
            }

            rtsp_s._f_list.erase(client_msg_list);
            // 不需要client_msg_list--

            //            printf("=====>_f_list size = %d\n", rtsp_s._f_list.size());
            if (!rtsp_s._f_list.empty())
            {
                printf("Number of remaining clients : %d!\n", rtsp_s._f_list.size());
                for (source_msg = rtsp_s.rtsp_session.begin(); source_msg != rtsp_s.rtsp_session.end(); source_msg++)
                {
                    for (client_is_empty = rtsp_s._f_list.begin(); client_is_empty != rtsp_s._f_list.end(); client_is_empty++)
                    {
                        if (source_msg->first == client_is_empty->session_map->get_source_name())
                            break;
                    }
                    if (client_is_empty == rtsp_s._f_list.end())
                        source_msg->second._play = false;
                }
            }
            else
            {
                printf("NO CLIENTS!\n");
                rtsp_s.LockFlag = false;
                for (source_msg = rtsp_s.rtsp_session.begin(); source_msg != rtsp_s.rtsp_session.end(); source_msg++)
                    source_msg->second._play = false;
            }
            //            rtsp_s.mutex.mutex_unlock();
            // 给所有的的资源解锁
            for (source_msg = rtsp_s.rtsp_session.begin(); source_msg != rtsp_s.rtsp_session.end(); source_msg++)
                source_msg->second.s_mutex.mutex_unlock();
            printf("Clean up the client thread!\n");
            if (client_msg_list->session_pid > 0)
                pthread_cancel(client_msg_list->session_pid);
        }
    }

    return 0;
}

/*设置客户端的保活机制，心跳包*/
int RTSP_SESSION::set_keeplive(void)
{
//    clientfd = sock.get_tcp_fd();

    int keep_alive = 1;
    int keep_idle = 10;
    int keep_interval = 3;
    int keep_count = 3;

    if (setsockopt(sock.get_tcp_fd(), SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(keep_alive)))
    {
        perror("Error setsockopt(SO_KEEPALIVE) failed");
        return -1;
    }
    if (setsockopt(sock.get_tcp_fd(), IPPROTO_TCP, TCP_KEEPIDLE, &keep_idle, sizeof(keep_idle)))
    {
        perror("Error setsockopt(TCP_KEEPIDLE) failed");
        return -1;
    }
    if (setsockopt(sock.get_tcp_fd(), SOL_TCP, TCP_KEEPINTVL, (void *)&keep_interval, sizeof(keep_interval)))
    {
        perror("Error setsockopt(TCP_KEEPINTVL) failed");
        return -1;
    }
    if (setsockopt(sock.get_tcp_fd(), SOL_TCP, TCP_KEEPCNT, (void *)&keep_count, sizeof(keep_count)))
    {
        perror("Error setsockopt(TCP_KEEPCNT) failed");
        return -1;
    }

    return 0;
}

/*
 *函数描述：接收会话交互线程
 *参数：void
 *返回值：void
 */

void RTSP_SESSION::thread_proc(void)
{
    int ret = 0;
    int i = 0;

    set_keeplive();

    while (IsDestroy() == false)
    {
        fd_set rfd;
        fd_set efd;
        FD_ZERO(&rfd);
        FD_ZERO(&efd);
        FD_SET(sock.get_tcp_fd(), &rfd);
        FD_SET(sock.get_tcp_fd(), &efd);
        struct timeval timeout = {0L, 1000L};

        ret = select(sock.get_tcp_fd() + 1, &rfd, NULL, &efd, &timeout);
        if (ret > 0)
        {
            if (FD_ISSET(sock.get_tcp_fd(), &rfd) != 0)
            {
                if ((ret = recv_data()) < 0)
                {
                    if(ret == -2)
                    {
                        printf("The client is disconnected abnormally. Check the cause!\n");
                        break;
//                        clear_source(sock.get_tcp_fd());
                    }
                }
            }
            if (FD_ISSET(sock.get_tcp_fd(), &efd) != 0)
            {
                printf("====>client error fd = %d, error = %d\n", sock.get_tcp_fd(), errno);
                perror("read client data error!");
                break;
            }
        }
#if 0
        uint8_t buf[1024] = {0};
        ret = recv(sock.get_tcp_fd(), buf, 1, MSG_PEEK);
        if (ret <= 0)
        {
            printf("listen client error!\n");
        }
        printf("ret = %d i = %d, error = %d, %d\n", ret, i++, errno, ETIMEDOUT);
#endif
    }
    // 该线程结束，说明客户端连接断开，清除掉该客户端的资源
    clear_source(sock.get_tcp_fd());

    return;
}

/*
 *函数描述：开始创建会话线程
 *参数：
 *   fd: 客户端文件描述符
 *   ip: 客户端ip
 *返回值：
 *   void
 */
void RTSP_SESSION::start(int fd, uint8_t *ip)
{
    pthread_t pid_t;

    printf("start session pthread!\n");
    sock.set_tcp_fd(fd);
    sock.set_client_ip(ip);

    create_pth("RtspSession");

    pid_t = get_pthread_pid();

    RTSP_S rtsp_s;

    std::list<CLI_MSG>::iterator client_msg_list;
    for (client_msg_list = rtsp_s._f_list.begin(); client_msg_list != rtsp_s._f_list.end(); client_msg_list++)
    {
        if (client_msg_list->cli_tcp_fd == fd)
            client_msg_list->session_pid = pid_t;
    }

    return;
}

/*
 *函数描述：接收客户端信息
 *参数：void
 *返回值：
 *   void
 *
 */
int RTSP_SESSION::recv_data(void)
{

    int ret = 0;
    unsigned char m_recv_buf[MAX_RECV_BUF_LEN] = {0};

    ret = sock.Recv(m_recv_buf, sizeof(m_recv_buf));
    if (ret < 0)
    {
//        printf("sock fd = %d\n", sock.get_tcp_fd());
        printf("recv client data error\n");
        if(ret == -2)
            return -2;
        else
            return -1;
    }
    printf("recv client data : \n");
    printf("%s\n", m_recv_buf);
    return handle_data((const char *)m_recv_buf);
}

/*
 *函数描述：解析客户端发来的交互信息
 *参数：
 *   data：接收到的客户端信息
 *返回值 成功 0 失败 -1
 */

int RTSP_SESSION::get_method(const char *data)
{
    int method = RTSP_METHOD_MAX;

    if ((*data == 'O') && (strncmp(data, s_method[RTSP_OPTIONS].method_str, strlen(s_method[RTSP_OPTIONS].method_str)) == 0))
        method = RTSP_OPTIONS;
    else if ((*data == 'D') && (strncmp(data, s_method[RTSP_DESCRIBE].method_str, strlen(s_method[RTSP_DESCRIBE].method_str)) == 0))
        method = RTSP_DESCRIBE;
    else if ((*data == 'S') && (strncmp(data, s_method[RTSP_SETUP].method_str, strlen(s_method[RTSP_SETUP].method_str)) == 0))
        method = RTSP_SETUP;
    else if ((*data == 'P') && (strncmp(data, s_method[RTSP_PLAY].method_str, strlen(s_method[RTSP_PLAY].method_str)) == 0))
        method = RTSP_PLAY;
    else if ((*data == 'P') && (strncmp(data, s_method[RTSP_PAUSE].method_str, strlen(s_method[RTSP_PAUSE].method_str)) == 0))
        method = RTSP_PAUSE;
    else if ((*data == 'T') && (strncmp(data, s_method[RTSP_TEARDOWN].method_str, strlen(s_method[RTSP_TEARDOWN].method_str)) == 0))
        method = RTSP_TEARDOWN;
    else if ((*data == 'S') && (strncmp(data, s_method[RTSP_SET_PARAMETER].method_str, strlen(s_method[RTSP_SET_PARAMETER].method_str)) == 0))
        method = RTSP_SET_PARAMETER;
    else if ((*data == 'G') && (strncmp(data, s_method[RTSP_GET_PARAMETER].method_str, strlen(s_method[RTSP_GET_PARAMETER].method_str)) == 0))
        method = RTSP_GET_PARAMETER;
    _method = method;

    return method;
}

/*
 *函数描述：字符串提取
 *
 */

void RTSP_SESSION::get_str(const char *data, const char *s_mark, const char *e_mark, char *dest, int dest_len)
{
    char *start = NULL;
    char *end = NULL;

    if (s_mark != NULL)
    {
        start = strstr((char *)data, s_mark);
        if (start != NULL)
        {
            if (e_mark != NULL)
                end = strstr(start, e_mark);
            if (end != NULL)
            {
                strncpy(dest, start, ((end - start) > dest_len) ? dest_len : (end - start));
            }
        }
    }

    return;
}

/*
 *函数描述：获取会话的seq值
 *
 */
int RTSP_SESSION::get_cseq(const char *data)
{
    memset(_CSeq, 0, sizeof(_CSeq));

    if (data != NULL)
    {
        get_str(data, "CSeq: ", "\r\n", _CSeq, sizeof(_CSeq) - 1 - strlen("\r\n"));

        strncpy(_CSeq + strlen(_CSeq), "\r\n", strlen("\r\n"));
    }
    else
    {
        std::cout << "data const is NULL" << std::endl;
        return -1;
    }

    return 0;
}

/*
 *函数描述符：去除字符串中的空格
 */
void RTSP_SESSION::remove_space(char *buf, int len)
{
    int i = 0;
    int j = 0;
    char *data = (char *)malloc(len);
    int new_len = len;

    for (i = 0; i < new_len; i++)
    {
        if (buf[i] == 0x20)
        {
            if (i < (new_len - 1))
            {
                for (j = i; j + 1 < new_len; j++)
                    buf[j] = buf[j + 1];
                i--;
                new_len -= 1;
            }
            if (i == new_len - 1)
                new_len -= 1;
        }
    }
    memcpy(data, buf, new_len);
    memset(buf, 0, len);
    memcpy(buf, data, new_len);

    if (data)
    {
        free(data);
        data = NULL;
    }
}

/*
 *函数描述：获取会话的url地址
 *
 */
int RTSP_SESSION::get_url(const char *data)
{
    memset(_rtsp_url, 0, sizeof(_rtsp_url));

    if (data != NULL)
    {
        get_str(data, "rtsp://", "RTSP", _rtsp_url, sizeof(_rtsp_url) - 1);
        remove_space(_rtsp_url, strlen(_rtsp_url));
    }
    else
    {
        std::cout << "data const is NULL" << std::endl;
        return -1;
    }

    return 0;
}

/*
 *函数描述：获取会话的版本
 *
 */
int RTSP_SESSION::get_version(const char *data)
{
    memset(_rtsp_ver, 0, sizeof(_rtsp_ver));

    if (data != NULL)
    {
        get_str(data, "RTSP", "\r\n", _rtsp_ver, sizeof(_rtsp_ver) - 1 - strlen("\r\n"));
        //        strncpy(_rtsp_ver+strlen(_rtsp_ver), "\r\n", strlen("\r\n"));
    }
    else
    {
        std::cout << "data const is NULL" << std::endl;
        return -1;
    }

    return 0;
}
/*
 *函数描述：发送回应的指令
 *参数：
 *   code：要回复的指令
 *返回值：void
 */
void RTSP_SESSION::send_simple_cmd(int code)
{
    int i = 0;
    unsigned char cmd_data[512] = {0};

    for (i = 0; i < sizeof(s_code); i++)
        if (s_code[i].code == code)
            break;

    sprintf((char *)cmd_data, "%s %d %s\r\n%s%s\r\n", _rtsp_ver, s_code[i].code, s_code[i].code_str, _CSeq, _session);

    sock.Send(cmd_data, strlen((char *)cmd_data));

    return;
}

void RTSP_SESSION::send_cmd(unsigned char *cmd, int cmd_len)
{
    printf("start send replay data!\n");
    printf("%s\n", cmd);
    sock.Send(cmd, cmd_len);

    return;
}

void RTSP_SESSION::handle_options(void)
{

    unsigned char cmd[512] = {0};
    char pub[128] = {0};

    strcpy(pub, "public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN");

    sprintf((char *)cmd, "%s 200 OK\r\n%s%s%s\r\n\r\n", _rtsp_ver, _CSeq, _session, pub);

    send_cmd(cmd, strlen((char *)cmd));

    return;
}
/*
 *函数描述：处理交互中的DESCRIBE指令
 *
 */
void RTSP_SESSION::handle_describe()
{
    unsigned char cmd[1024] = {0};
    int ret = 0;

    source_name = strstr(strstr(_rtsp_url, "rtsp://") + strlen("rtsp://"), "/") + 1;

    ret = sdp.init_sdp(strstr(strstr(_rtsp_url, "rtsp://") + strlen("rtsp://"), "/") + 1, sock.get_tcp_fd());
    if (ret < 0)
    {
        clear_source(sock.get_tcp_fd());
        return send_simple_cmd(400);
    }

    const char *sdp_s = NULL;
    sdp_s = sdp.get_sdp();

    sprintf((char *)cmd,
            "%s 200 OK\r\n"
            "%s%s"
            "Date: 12 Jan 2023 15:44:23 GMT\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: %d\r\n\r\n"
            "%s",
            _rtsp_ver, _CSeq, _session, strlen(sdp_s), sdp_s);

    send_cmd(cmd, strlen((char *)cmd));

    return;
}

uint64_t RTSP_SESSION::GetTime()
{
    struct timeval cur_time;
    gettimeofday(&cur_time, NULL);
    return cur_time.tv_sec * 1000000 + cur_time.tv_usec;
}

/*
 *函数描述：处理交互中的SETUP指令
 *
 */
void RTSP_SESSION::handle_setup(const char *data)
{
    int in_data = 0;
    char *interleaved = NULL;
    unsigned char cmd[1024] = {0};
    char *rtcp_pot = NULL;
    char *streamId = NULL;
    int stream_id = 0;
    int video_flag = 0;
    int audio_flag = 0;

    if (strstr((char *)data, "RTP/AVP") == NULL)
        return send_simple_cmd(461);

    if ((streamId = strstr((char *)data, "track0")) != NULL)
    {
        if ((interleaved = strstr((char *)data, "client_port=")) == NULL)
        {
            return send_simple_cmd(400);
        }
        video_rtpport = atoi(interleaved + strlen("client_port="));
        if (video_rtpport < 0)
        {
            return send_simple_cmd(400);
        }
        rtcp_pot = strstr(interleaved, "-");
        video_rtcpport = atoi(rtcp_pot + strlen("-"));

        sock.set_video_rtp_port(video_rtpport);
        sock.set_video_rtcp_port(video_rtcpport);
        //        sprintf(_session, "Session: 5AC2E264\r\n");
        sprintf(_session, "Session: %x\r\n", time(NULL));
        sprintf((char *)cmd,
                "%s 200 OK\r\n"
                "%s%s"
                "Transport: RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d\r\n\r\n",
                _rtsp_ver, _CSeq, _session, video_rtpport, video_rtpport + 1, SERVERRTPPORT, SERVERRTCPPORT);
    }
    if ((streamId = strstr((char *)data, "track1")) != NULL)
    {
        if ((interleaved = strstr((char *)data, "client_port=")) == NULL)
        {
            return send_simple_cmd(400);
        }
        audio_rtpport = atoi(interleaved + strlen("client_port="));
        if (audio_rtpport < 0)
        {
            return send_simple_cmd(400);
        }
        rtcp_pot = strstr(interleaved, "-");
        audio_rtcpport = atoi(rtcp_pot + strlen("-"));

        sock.set_audio_rtp_port(audio_rtpport);
        sock.set_audio_rtcp_port(audio_rtcpport);
        sprintf((char *)cmd,
                "%s 200 OK\r\n"
                "%s%s"
                "Transport: RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d\r\n\r\n",
                _rtsp_ver, _CSeq, _session, audio_rtpport, audio_rtpport + 1, AUDIO_S_RTPPORT, AUDIO_S_RTCPPORT);
    }

    send_cmd(cmd, strlen((char *)cmd));

    return;
}

/*
 *函数描述：处理交互中的PLAY指令
 *
 */

void RTSP_SESSION::handle_play(const char *data)
{
    unsigned char cmd[1024] = {0};
#if 0
    int range = sdp.get_file_range();
    int s_sec = 0;
	int e_sec = range;
	const char* range_s = strstr( data, "Range: npt=" );
	if( range_s != NULL ){
		s_sec = atoi( range_s+strlen("Range: npt=") );
		const char* e_sec_pos = strstr( range_s+strlen("Range: npt="), "-" );
		if( e_sec_pos != NULL ){
			int sec = atoi( e_sec_pos+1 );
			if( sec != 0 )
				e_sec = sec;
		}
	}
#endif
    sprintf((char *)cmd,
            "RTSP/1.0 200 OK\r\n"
            "%s"
            "Range: npt=0.000-\r\n"
            "%s"
            "\r\n",
            _CSeq, _session);

    send_cmd(cmd, strlen((char *)cmd));

    isPlay = 1;
    _play = true;
    //   printf("this client fd = %d\n", sock.get_tcp_fd());
    //    sdp.play(&sock, m_rtp_ch);
    return;
}

void RTSP_SESSION::handle_pause()
{
    return;
}

void RTSP_SESSION::handle_set_parameter()
{
    return;
}

void RTSP_SESSION::handle_get_parameter()
{
    return;
}
/*
 *函数描述：处理交互中的TEARDOWN指令
 *
 */
void RTSP_SESSION::handle_teardown(void)
{
    char cmd[1024] = {0};

    sprintf(cmd,
            "%s 200 OK\r\n"
            "%s%s",
            _rtsp_ver, _CSeq, _session);

    clear_source(sock.get_tcp_fd());

    return;
}

/*
 *函数描述：处理交互的数据指令
 *参数：
 *   data:客户端数据
 *返回值：void
 */
int RTSP_SESSION::handle_data(const char *data)
{
    int method = RTSP_METHOD_MAX;
    int ret = 0;

    ret = get_version(data);
    if (ret < 0)
    {
        std::cout << "get version failed" << std::endl;
        send_simple_cmd(400);
        return -1;
    }
    ret = get_cseq(data);
    if (ret < 0)
    {
        std::cout << "get cseq failed" << std::endl;
        send_simple_cmd(400);
        return -1;
    }
    ret = get_url(data);
    if (ret < 0)
    {
        std::cout << "get url failed" << std::endl;
        send_simple_cmd(400);
        return -1;
    }

    method = get_method(data);
    if (method == RTSP_METHOD_MAX)
    {
        std::cout << "get RTCP CMD!" << std::endl;
        return -1;
    }

    switch (method)
    {
    case RTSP_OPTIONS:
        printf("get Options messege!\n");
        handle_options();
        break;
    case RTSP_DESCRIBE:
        printf("get Describe messege!\n");
        handle_describe();
        break;
    case RTSP_SETUP:
        printf("get Setup messege!\n");
        handle_setup(data);
        break;
    case RTSP_PLAY:
        printf("get Play messege!\n");
        handle_play(data);
        break;
    case RTSP_TEARDOWN:
        printf("get Teardown messege\n");
        handle_teardown();
        break;
    case RTSP_PAUSE:
        handle_pause();
        break;
    case RTSP_SET_PARAMETER:
        handle_set_parameter();
        break;
    case RTSP_GET_PARAMETER:
        handle_get_parameter();
        break;
    }

    return 0;
}