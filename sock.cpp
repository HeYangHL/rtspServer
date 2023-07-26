#include "sock.hpp"
#include "rtspserver.hpp"



int SOCK_T::serverRtpSockfd = 0;
int SOCK_T::serverRtcpSockfd = 0;
int SOCK_T::serverAudioRtpSockfd = 0;
int SOCK_T::serverAudioRtcpSockfd = 0;

#if 0
int SOCK_T::clientRtpPort = 0;
int SOCK_T::clientRtcpPort = 0;
uint8_t SOCK_T::client_ip[16] = {0};
#endif

/*
*函数描述：TCP创建套接字
*参数：void
*返回值：成功 0 失败 -1
*
*/
int SOCK_T::open_sock(void)
{
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(tcp_fd == -1)
    {
        perror("open sock error");
        return -1;
    }

    return 0;
}

/*
*函数描述：设置套接字属性
*参数：void
*返回值：成功 0 失败 -1
*
*/
int SOCK_T::set_sock_opt(void)
{
    int opt = 1;
    int ret = 0;

    ret = setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
    if(ret < 0)
    {
        perror("set sock opt error");
        return -1;
    }

    return 0;
}
/*
*函数描述：绑定套接字
*参数：void
*返回值：成功 0 失败 -1
*/
int SOCK_T::bind_sock(void)
{
    struct sockaddr_in sin;
    int ret = 0;

    bzero(&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(server_port);
    sin.sin_addr.s_addr = inet_addr(server_ip.c_str());

    ret = bind(tcp_fd, (struct sockaddr *)&sin, sizeof(sin));
    if(ret < 0)
    {
        perror("bind server ip error");
        return -1;
    }

    return 0;
}

/*
*函数描述：监听套接字
*参数：void
*返回值：成功 0 失败 -1
*/
int SOCK_T::listen_sock(void)
{
    int ret = 0;

    ret = listen(tcp_fd, 10);
    if(ret < 0)
    {
        perror("listen error!");
        return -1;
    }
    create_pth("Rtspsver");
    tcp_pid = get_pthread_pid();

    return 0;
}

/*
*函数描述：创建tcp监视线程，监视是否有客户端连接
*参数：void
*返回值：void
*
*/
void SOCK_T::thread_proc(void)
{
    int ret = 0;
    int c_fd = 0;
    RTSP_S rtsp_s;
    
    while(IsDestroy() == false)
    {
        fd_set rfd;
        fd_set efd;

        FD_ZERO(&rfd);
        FD_ZERO(&efd);
        FD_SET(tcp_fd, &rfd);
        FD_SET(tcp_fd, &efd);
        struct timeval timeout = {0L, 1000L};

        rtsp_s.creat_media_pthread();
        ret = select(tcp_fd+1, &rfd, NULL, &efd, &timeout);
        if(ret > 0)
        {
            if(FD_ISSET(tcp_fd, &rfd) != 0)
            {
                struct sockaddr_in remote_fin;
                socklen_t sock_size = sizeof(struct sockaddr_in);
                c_fd = accept(tcp_fd, (struct sockaddr *)&remote_fin, &sock_size);
                if(c_fd > 0)
                {   
                    printf("client connect : ip : %s, port : %d, fd = %d\n", inet_ntoa(remote_fin.sin_addr), remote_fin.sin_port, c_fd);
                    strcpy((char *)client_ip, inet_ntoa(remote_fin.sin_addr));
                    clientPort = ntohs(remote_fin.sin_port);
                    accept_sock(c_fd, client_ip);
                    #if 1
                    if(!rtsp_s.LockFlag)
                    {
                        rtsp_s.source_mutex.mutex_lock();
 //                       rtsp_s.source_mutex.mutex_cond_signal();
                        rtsp_s.source_mutex.mutex_cond_broadcast();
                        rtsp_s.source_mutex.mutex_unlock();
                        rtsp_s.LockFlag = true;
                    }
                    #endif
                }
            }
            if(FD_ISSET(tcp_fd, &efd) != 0)
            {
                printf("server tcp error fd = %d\n", tcp_fd);
                printf("client connect server error!\n");
                
                return;
            }
            
        }
        
 
    }

    return;
}

/*
*函数描述：接收客户端数据
*参数：
*   buf : 客户端数据缓存
*   len ：数据长度
*返回值：
*   成功 0 失败 -1
*/
int SOCK_T::Recv(const unsigned char *buf, int len)
{
    int ret = 0;

    ret = recv(tcp_fd, (char *)buf, len, 0);
    if(ret < 0)
    {
        if( ERROR_NO == N_EWOULDBLOCK || ERROR_NO == N_EINTR || ERROR_NO == N_EAGAIN )
			return 0;

        perror("recv error !");
		return -1;
    }else if( ret == 0  || errno == ETIMEDOUT)
    {
        perror("recv error !");
		return -2;
    }
    return ret;
}


/*
*函数描述：发送客户端数据
*参数：
*   buf：客户端数据
*   len：发送的长度
*返回值：
*   成功 0 失败 -1
*/
int SOCK_T::Send(const unsigned char* buf, int len)
{
	if( buf == NULL || len <= 0)
		return -1;
	int ret = send( tcp_fd, (char *)buf, len, 0);
	if( ret < 0 ){
		if( ERROR_NO == N_EWOULDBLOCK || ERROR_NO == N_EINTR || ERROR_NO == N_EAGAIN )
			return 0;
		return -1;
	}
	return ret;
}

/*
*函数描述：发送rtp数据包
*参数：
*   type：流媒体类型
*   buf：要发送的rtp包
*   len：发送的数据长度
*返回值：
*   成功 0 失败 -1
*/
int SOCK_T::SendTo(std::string type, const unsigned char* buf, int len)
{
    struct sockaddr_in addr;
    int ret;

    if(type == "audio")
    {       
        addr.sin_family = AF_INET;
        addr.sin_port = htons(audioRtpPort);
        addr.sin_addr.s_addr = inet_addr((char *)client_ip);
        ret = sendto(serverAudioRtpSockfd, buf, len, 0,(struct sockaddr*)&addr, sizeof(addr));
    }
    if(type == "video")
    {
        addr.sin_family = AF_INET;
        addr.sin_port = htons(videoRtpPort);
        addr.sin_addr.s_addr = inet_addr((char *)client_ip);
        ret = sendto(serverRtpSockfd, buf, len, 0,(struct sockaddr*)&addr, sizeof(addr));
    }
    return ret;
}

/*
*函数描述：发送rtcp数据包
*参数：
*   type：流媒体类型
*   buf：要发送的rtcp包
*   len：发送的数据长度
*返回值：
*   成功 0 失败 -1
*/
int SOCK_T::SendRtcpTo(std::string type, const unsigned char* buf, int len)
{
    struct sockaddr_in addr;
    int ret;

//    printf("audioRtcpPort = %d, audioRtpPort = %d,videoRtcpPort = %d, videoRtpPort = %d\n", audioRtcpPort, audioRtpPort, videoRtcpPort, videoRtpPort);
    if(type == "audio")
    {       
        addr.sin_family = AF_INET;
        addr.sin_port = htons(audioRtcpPort);
        addr.sin_addr.s_addr = inet_addr((char *)client_ip);
        ret = sendto(serverAudioRtcpSockfd, buf, len, 0,(struct sockaddr*)&addr, sizeof(addr));
    }
    if(type == "video")
    {   
        addr.sin_family = AF_INET;
        addr.sin_port = htons(videoRtcpPort);
        addr.sin_addr.s_addr = inet_addr((char *)client_ip);
        ret = sendto(serverRtcpSockfd, buf, len, 0,(struct sockaddr*)&addr, sizeof(addr));
    }
    return ret;
}

/*绑定服务器端视频rtp端口的套接字*/
int SOCK_T::bindRtpSocketAddr()
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVERRTPPORT);
    addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

    if(bind(serverRtpSockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
        return -1;

    return 0;
}
/*绑定服务器端视频rtcp端口的套接字*/
int SOCK_T::bindRtcpSocketAddr()
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVERRTCPPORT);
    addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

    if(bind(serverRtcpSockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
        return -1;

    return 0;
}
/*绑定服务器端音频rtp端口的套接字*/
int SOCK_T::bindAudioRtpSocketAddr()
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(AUDIO_S_RTPPORT);
    addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

    if(bind(serverAudioRtpSockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
        return -1;

    return 0;
}
/*绑定服务器端音频rtp端口的套接字*/
int SOCK_T::bindAudioRtcpSocketAddr()
{
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(AUDIO_S_RTCPPORT);
    addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

    if(bind(serverAudioRtcpSockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
        return -1;

    return 0;
}



/*创建服务器端视频rtp端口的套接字*/
int SOCK_T::createRtpSocket()
{
    int on = 1;

    
    serverRtpSockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(serverRtpSockfd < 0)
        return -1;

    setsockopt(serverRtpSockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

    on = 1;
    setsockopt(serverRtpSockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&on, sizeof(on));
    return serverRtpSockfd;
}
/*创建服务器端视频rtcp端口的套接字*/
int SOCK_T::createRtcpSocket()
{
    int on = 1;
     
    serverRtcpSockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(serverRtcpSockfd < 0)
        return -1;

    setsockopt(serverRtcpSockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));
    return serverRtcpSockfd;
}
/*创建服务器端音频rtp端口的套接字*/
int SOCK_T::createAudioRtpSocket()
{
    int on = 1;

    
    serverAudioRtpSockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(serverAudioRtpSockfd < 0)
        return -1;

    setsockopt(serverAudioRtpSockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

    on = 1;
    setsockopt(serverAudioRtpSockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&on, sizeof(on));
    return serverAudioRtpSockfd;
}
/*创建服务器端音频rtcp端口的套接字*/
int SOCK_T::createAudioRtcpSocket()
{
    int on = 1;
     
    serverAudioRtcpSockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(serverAudioRtcpSockfd < 0)
        return -1;

    setsockopt(serverAudioRtcpSockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));
    return serverAudioRtcpSockfd;
}



