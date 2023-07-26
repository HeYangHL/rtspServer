#include "r_media_stream.hpp"
#include "rtspserver.hpp"


/*
*函数描述：创建媒体流发送线程
*参数：void
*返回值：void
*
*/
void MEDIA_STREAM::start_media(void)
{
	RTSP_S rtsp_s;
	int i = 0;
	struct timeval now;

	std::map<std::string, SOURCE>::iterator it;

	rtsp_s.source_mutex.mutex_lock();
	if(rtsp_s.rtsp_session.size() == 0)
		return;
	
	for (it = rtsp_s.rtsp_session.begin(); it != rtsp_s.rtsp_session.end(); it++)
	{
		if(it->second.pthread_flag == 0)
		{
			create_pth(it->first);
			gettimeofday(&now, NULL);
			it->second.ssrc = now.tv_sec * 1000000ULL + now.tv_usec;
//			printf("====>ssrc = %x, i = %d\n",it->second.ssrc, i++);
			it->second.pid = get_pthread_pid();
//			printf("creat pthread name : %s, pid = %08x\n", it->first.c_str(), it->second.pid);
			usleep(1000);
			it->second.pthread_flag = 1;
		}
	}
	rtsp_s.source_mutex.mutex_unlock();

	return;
}

/*
*函数描述：rtp载荷AAC的前四字节描述信息
*参数：
*	buf：存放四字节的空间
*	len：aac帧长度
*返回值：成功 0
*/
int MEDIA_STREAM::four_payload(uint8_t *buf, int len)
{
	buf[0] = 0x0;
	buf[1] = 0x10;
	buf[2] = (len & 0x1fe0) >> 5;
	buf[3] = (len & 0x1f) << 3;

	return 0;
}

/*
*函数描述：rtsp发送rtp载荷aac的rtp包
*参数：
*	sock：该会话的sock对象
*	rtpPacket: rtp头数据
*	fram：数据帧
*	frameSize：帧大小
*	RtcpChar：发送的rtp总数
*返回值：
*	成功 0 失败 -1
*/
int MEDIA_STREAM::rtpSendAACFrame(SOCK_T *sock, RtpHdr *rtpPacket, uint8_t *frame, uint32_t frameSize, uint32_t &RtcpChar)
{
	uint8_t *rtpdata = NULL;
	uint8_t fort_B[4] = {0};
	int len = 0;
	int ret = 0;

	rtpdata = new uint8_t[frameSize + sizeof(RtpHdr) + 4];
	memset(rtpdata, 0, frameSize + sizeof(RtpHdr));
	host_to_net(rtpPacket);
	memcpy(rtpdata + len, rtpPacket, sizeof(RtpHdr));
	net_to_host(rtpPacket);
	len += sizeof(RtpHdr);

	if (frameSize <= 1400)
	{
		four_payload(fort_B, frameSize);

		memcpy(rtpdata + len, fort_B, sizeof(fort_B));

		len += sizeof(fort_B);
		memcpy(rtpdata + len, frame, frameSize);
		len += frameSize;
#if 0
		printf("RTP AAC framesize = %d, len = %d\n", frameSize, len);
		printf("RTP AAC data:\n");
		for(int i = 0; i < 64; i++)
		{
			if(i%16 == 0)
				printf("\n");
			printf("%02x ", rtpdata[i]);
		}
		printf("\n");
#endif
		ret = rtpSendPacket(sock, "audio", rtpdata, len);
		if (ret < 0)
		{
			printf("send AAC rtp pack error!\n");
			return -1;
		}
		RtcpChar++;
		rtpPacket->seq++;
	}

	if (rtpdata)
	{
		delete[] rtpdata;
		rtpdata = NULL;
	}

	return len;
}

/*
*函数描述：初始化视频rtp头
*参数：
*	rtp_head：rtp头数据
*	ssrc：媒体流对应的ssrc
*返回值：
*	成功 0
*/
int MEDIA_STREAM::v_rtp_head_init(RtpHdr *rtp_head, uint32_t ssrc)
{
	rtp_head->version = 2;
	rtp_head->p = 0;
	rtp_head->x = 0;
	rtp_head->cc = 0;
	rtp_head->m = 0;
	rtp_head->pt = 96;
	rtp_head->seq = 0;
	rtp_head->ts = 0;
	rtp_head->ssrc = ssrc+1;
}

/*
*函数描述：初始化音频rtp头
*参数：
*	rtp_head：rtp头数据
*	ssrc：媒体流对应的ssrc
*返回值：
*	成功 0
*/
int MEDIA_STREAM::a_rtp_head_init(RtpHdr *rtp_head, uint32_t ssrc)
{
	rtp_head->version = 2;
	rtp_head->p = 0;
	rtp_head->x = 0;
	rtp_head->cc = 0;
	rtp_head->m = 1;
	rtp_head->pt = 97;
	rtp_head->seq = 0;
	rtp_head->ts = 0;
	rtp_head->ssrc = ssrc+2;

}

/*
*函数描述：初始化视频rtcp头
*参数：
*	rtcp_head：rtcp头数据
*	ssrc：媒体流对应的ssrc
*返回值：
*	成功 0
*/
int MEDIA_STREAM::v_rtcp_head_init(RtcpHdr *rtcp_head, uint32_t ssrc)
{
	rtcp_head->v = 2;
	rtcp_head->p = 0;
	rtcp_head->rc = 0;
	rtcp_head->pt = 200;
	rtcp_head->ssrc = ssrc+1;
	rtcp_head->lenght = 0x6;

	return 0;
}


/*
*函数描述：初始化音频rtcp头
*参数：
*	rtcp_head：rtcp头数据
*	ssrc：媒体流对应的ssrc
*返回值：
*	成功 0
*/
int MEDIA_STREAM::a_rtcp_head_init(RtcpHdr *rtcp_head, uint32_t ssrc)
{
	rtcp_head->v = 2;
	rtcp_head->p = 0;
	rtcp_head->rc = 0;
	rtcp_head->pt = 200;
	rtcp_head->ssrc = ssrc+2;
	rtcp_head->lenght = 0x6;

	return 0;
}

/*
*函数描述：获取AAC的adts头的个数
*参数：
*	frame：音频帧
*返回值：
*	成功 0
*/
int MEDIA_STREAM::get_adts_bye(uint8_t *frame)
{
	int adtsSize = 0;

	if ((frame[1] & 0x1) == 1)
		return 7;
	if ((frame[1] & 0x1) == 0)
		return 9;
}


/*
*函数描述：rtp发送包
*
*
*
*/
int MEDIA_STREAM::rtpSendPacket(SOCK_T *m_sock, std::string type, uint8_t *frame_data, uint32_t dataSize)
{
	int ret = 0;

	if (m_sock != NULL)
	{
		if ((ret = m_sock->SendTo(type, frame_data, dataSize)) < 0)
		{
			printf("send RTP packet error!\n");
			return -1;
		}
		return ret;
	}
	return -1;
}

/*函数描述：发送rtcp包*/
int MEDIA_STREAM::rtcpSendPacket(SOCK_T *m_sock, std::string type, uint8_t *frame_data, uint32_t dataSize)
{
	int ret = 0;

	if (m_sock != NULL)
	{
		if ((ret = m_sock->SendRtcpTo(type, frame_data, dataSize)) < 0)
		{
			printf("send RTP packet error!\n");
			return -1;
		}
		return ret;
	}
	return -1;
}

/*将rtp头的网络字节序转成主机字节序*/
void MEDIA_STREAM::net_to_host(RtpHdr *rtpPacket)
{

	rtpPacket->seq = ntohs(rtpPacket->seq);
	rtpPacket->ts = ntohl(rtpPacket->ts);
	rtpPacket->ssrc = ntohl(rtpPacket->ssrc);

	return;
}

/*将rtp头的主机字节序转成网络字节序*/
void MEDIA_STREAM::host_to_net(RtpHdr *rtpPacket)
{
	rtpPacket->seq = htons(rtpPacket->seq);
	rtpPacket->ts = htonl(rtpPacket->ts);
	rtpPacket->ssrc = htonl(rtpPacket->ssrc);

	return;
}

/*将rtcp头的网络字节序转成主机字节序*/
void MEDIA_STREAM::rtcp_net_to_host(RtcpHdr *rtcpPacket)
{
	rtcpPacket->lenght = ntohs(rtcpPacket->lenght);
	rtcpPacket->ssrc = ntohs(rtcpPacket->ssrc);
	rtcpPacket->ntp_sec = ntohs(rtcpPacket->ntp_sec);
	rtcpPacket->ntp_usec = ntohs(rtcpPacket->ntp_usec);
	rtcpPacket->pts = ntohs(rtcpPacket->pts);
	rtcpPacket->p_count = ntohs(rtcpPacket->p_count);
	rtcpPacket->c_count = ntohs(rtcpPacket->c_count);

	return;
}

/*将rtcp头的主机字节序转成网络字节序*/
void MEDIA_STREAM::rtcp_host_to_net(RtcpHdr *rtcpPacket)
{
	rtcpPacket->lenght = htons(rtcpPacket->lenght);
	rtcpPacket->ssrc = htonl(rtcpPacket->ssrc);
	rtcpPacket->ntp_sec = htonl(rtcpPacket->ntp_sec);
	rtcpPacket->ntp_usec = htonl(rtcpPacket->ntp_usec);
	rtcpPacket->pts = htonl(rtcpPacket->pts);
	rtcpPacket->p_count = htonl(rtcpPacket->p_count);
	rtcpPacket->c_count = htonl(rtcpPacket->c_count);

	return;
}

/*填充并且发送rtcp控制包*/
int MEDIA_STREAM::rtcpSendControl(SOCK_T *sock, std::string type, RtcpHdr rtcpPacket, uint64_t pNum, uint64_t pts, uint64_t pLen)
{
	struct timeval tv;
	uint8_t *buf = NULL;

	buf = new uint8_t[sizeof(RtcpHdr)];
	gettimeofday(&tv, NULL);

	uint64_t time_usec = tv.tv_sec * 1000000ULL + tv.tv_usec;
	uint64_t temp = (time_usec / 1000ULL) * 1000ULL + (2208988800ULL * 1000000ULL);

	rtcpPacket.ntp_sec = (uint32_t)(temp / 1000000ULL);
	rtcpPacket.ntp_usec = (uint32_t)(((temp % 1000000) << 32) / 1000000);
	rtcpPacket.pts = pts;
	rtcpPacket.p_count = pNum;
	rtcpPacket.c_count = pLen;

	rtcp_host_to_net(&rtcpPacket);
	memcpy(buf, &rtcpPacket, sizeof(RtcpHdr));
	rtcp_net_to_host(&rtcpPacket);

	rtcpSendPacket(sock, type, buf, sizeof(RtcpHdr));

	if (buf)
	{
		delete[] buf;
		buf = NULL;
	}
}

/*填充并且发送H264 rtp数据包*/

int MEDIA_STREAM::rtpSendH264Frame(SOCK_T *sock, RtpHdr *rtpPacket, uint8_t *frame, uint32_t frameSize, uint32_t &RtcpChar)
{
	uint8_t naluType; // nalu第一个字节
	int sendBytes = 0;
	int ret;

	uint8_t *frame_data = NULL;

	naluType = frame[0];
//	printf("=====>nalutype = %02x\n", naluType);

	//	printf("send frame data :\n");
	//	print_data(frame, 32);
	if (frameSize <= 1400) // nalu长度小于最大包场：单一NALU单元模式
	{
		/*
		 *   0 1 2 3 4 5 6 7 8 9
		 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *  |F|NRI|  Type   | a single NAL unit ... |
		 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 */

		rtpPacket->m = 1;
		frame_data = (uint8_t *)malloc(frameSize + sizeof(RtpHdr));
		memset(frame_data, 0, frameSize + sizeof(RtpHdr));
		host_to_net(rtpPacket);
		memcpy(frame_data, rtpPacket, sizeof(RtpHdr));
		net_to_host(rtpPacket);
		memcpy(frame_data + sizeof(RtpHdr), frame, frameSize);
		ret = rtpSendPacket(sock, "video", frame_data, frameSize + sizeof(RtpHdr));
		RtcpChar++;

		if (ret < 0)
			return -1;

		rtpPacket->seq++;
		sendBytes += ret;
		if (frame_data)
		{
			free(frame_data);
			frame_data = NULL;
		}
		if ((naluType & 0x1F) == 7 || (naluType & 0x1F) == 8) // 如果是SPS、PPS就不需要加时间戳
			goto out;
	}
	else // nalu长度小于最大包场：分片模式
	{
		/*
		 *  0                   1                   2
		 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
		 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 * | FU indicator  |   FU header   |   FU payload   ...  |
		 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 */

		/*
		 *     FU Indicator
		 *    0 1 2 3 4 5 6 7
		 *   +-+-+-+-+-+-+-+-+
		 *   |F|NRI|  Type   |
		 *   +---------------+
		 */

		/*
		 *      FU Header
		 *    0 1 2 3 4 5 6 7
		 *   +-+-+-+-+-+-+-+-+
		 *   |S|E|R|  Type   |
		 *   +---------------+
		 */

		int pktNum = frameSize / 1400;		  // 有几个完整的包
		int remainPktSize = frameSize % 1400; // 剩余不完整包的大小
		int i, pos = 1;
		char rtp_first = 0;
		char rtp_second = 0;

		/* 发送完整的包 */
		for (i = 0; i < pktNum; i++)
		{
			
			frame_data = (uint8_t *)malloc(1400 + sizeof(RtpHdr) + 2);
			memset(frame_data, 0, 1400 + sizeof(RtpHdr) + 2);

			if (remainPktSize == 0 && i == (pktNum - 1))
				rtpPacket->m = 1;

			rtpPacket->m = 0;
			host_to_net(rtpPacket);
			memcpy(frame_data, rtpPacket, sizeof(RtpHdr));
			net_to_host(rtpPacket);

			//            rtpPacket->payload[0] = (naluType & 0x60) | 28;
			//            rtpPacket->payload[1] = naluType & 0x1F;
			rtp_first = (naluType & 0x60) | 28;
			rtp_second = naluType & 0x1F;
			
			if (i == 0) // 第一包数据
				//                rtpPacket->payload[1] |= 0x80; // start
				rtp_second |= 0x80;
			else if (remainPktSize == 0 && i == pktNum - 1) // 最后一包数据
				//                rtpPacket->payload[1] |= 0x40; // end
				rtp_second |= 0x40;

			//            memcpy(rtpPacket->payload+2, frame+pos, 1400);
			
			memcpy(frame_data + sizeof(RtpHdr), &rtp_first, 1);
			memcpy(frame_data + sizeof(RtpHdr) + 1, &rtp_second, 1);
			memcpy(frame_data + sizeof(RtpHdr) + 2, frame + pos, 1400);
			ret = rtpSendPacket(sock, "video", frame_data, 1402 + sizeof(RtpHdr));
			RtcpChar++;

			if (ret < 0)
				return -1;

			rtpPacket->seq++;
			sendBytes += ret;
			pos += 1400;
			if (frame_data)
			{
				free(frame_data);
				frame_data = NULL;
			}
		}

		/* 发送剩余的数据 */
		if (remainPktSize > 0)
		{

			rtp_first = (naluType & 0x60) | 28;
			rtp_second = naluType & 0x1F;
			rtp_second |= 0x40; // end
			frame_data = (uint8_t *)malloc(remainPktSize + sizeof(RtpHdr) + 2);
			memset(frame_data, 0, remainPktSize + sizeof(RtpHdr) + 2);
			rtpPacket->m = 1;
			host_to_net(rtpPacket);
			memcpy(frame_data, rtpPacket, sizeof(RtpHdr));
			net_to_host(rtpPacket);
			memcpy(frame_data + sizeof(RtpHdr), &rtp_first, 1);
			memcpy(frame_data + sizeof(RtpHdr) + 1, &rtp_second, 1);
			memcpy(frame_data + sizeof(RtpHdr) + 2, frame + pos, remainPktSize);
			ret = rtpSendPacket(sock, "video", frame_data, remainPktSize + 2 + sizeof(RtpHdr));
			RtcpChar++;

			if (ret < 0)
				return -1;

			rtpPacket->seq++;
			sendBytes += ret;
			if (frame_data)
			{
				free(frame_data);
				frame_data = NULL;
			}
		}
	}

out:
	return sendBytes;
}
/*填充并且发送H265 rtp数据包*/
int MEDIA_STREAM::rtpSendH265Frame(SOCK_T *sock, RtpHdr *rtpPacket, uint8_t *frame, uint32_t frameSize, uint32_t &RtcpChar)
{
	uint16_t naluHead; // nalu第一个字节
	int sendBytes = 0;
	int ret;

	uint8_t *frame_data = NULL;

	naluHead = ((frame[0] << 8) | frame[1]);

	//	printf("send frame data :\n");
	//	print_data(frame, 32);

	if (frameSize <= 1400) // nalu长度小于最大包场：单一NALU单元模式
	{
		/*
		 *   0 1 2 3 4 5 6 7 8 9
		 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 *  |F|NRI|  Type   | a single NAL unit ... |
		 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		 */

		rtpPacket->m = 1;
		frame_data = (uint8_t *)malloc(frameSize + sizeof(RtpHdr));
		memset(frame_data, 0, frameSize + sizeof(RtpHdr));
		host_to_net(rtpPacket);
		memcpy(frame_data, rtpPacket, sizeof(RtpHdr));
		net_to_host(rtpPacket);
		memcpy(frame_data + sizeof(RtpHdr), frame, frameSize);
		ret = rtpSendPacket(sock, "video", frame_data, frameSize + sizeof(RtpHdr));		
		if (ret < 0)
			return -1;

		
		rtpPacket->seq++;
		RtcpChar++;
		sendBytes += ret;

		if (frame_data)
		{
			free(frame_data);
			frame_data = NULL;
		}
		if (((naluHead >> 9) & 0x3f) == 33 || ((naluHead >> 9) & 0x3f) == 34) // 如果是SPS、PPS就不需要加时间戳
			goto out;
	}
	else // nalu长度小于最大包场：分片模式
	{
		int pktNum = frameSize / 1400;		  // 有几个完整的包
		int remainPktSize = frameSize % 1400; // 剩余不完整包的大小
		int i, pos = 2;
		uint16_t rtp_ph = 0;
		uint8_t rtp_fu = 0;

		/* 发送完整的包 */
		for (i = 0; i < pktNum; i++)
		{
			frame_data = (uint8_t *)malloc(1400 + sizeof(RtpHdr) + 3);
			memset(frame_data, 0, 1400 + sizeof(RtpHdr) + 3);

			if (remainPktSize == 0 && i == (pktNum - 1))
				rtpPacket->m = 1;

			rtpPacket->m = 0;
			host_to_net(rtpPacket);
			memcpy(frame_data, rtpPacket, sizeof(RtpHdr));
			net_to_host(rtpPacket);

			rtp_ph = (((naluHead & 0x81ff) | (0x31 << 9)));
			rtp_fu = ((naluHead & 0x7e00) >> 9);

			if (i == 0) // 第一包数据
				rtp_fu |= 0x80;
			else if (remainPktSize == 0 && i == pktNum - 1) // 最后一包数据
				rtp_fu |= 0x40;

			rtp_ph = htons(rtp_ph);
			memcpy(frame_data + sizeof(RtpHdr), &rtp_ph, 2);
			memcpy(frame_data + sizeof(RtpHdr) + 2, &rtp_fu, 1);
			memcpy(frame_data + sizeof(RtpHdr) + 3, frame + pos, 1400);
			ret = rtpSendPacket(sock, "video", frame_data, 1403 + sizeof(RtpHdr));
			if (ret < 0)
				return -1;

			rtpPacket->seq++;
			RtcpChar++;
			sendBytes += ret;
			pos += 1400;

			if (frame_data)
			{
				free(frame_data);
				frame_data = NULL;
			}
		}

		/* 发送剩余的数据 */
		if (remainPktSize > 0)
		{

			rtp_ph = (((naluHead & 0x81ff) | (0x31 << 9)));
			rtp_fu = ((naluHead & 0x7e00) >> 9);
			rtp_fu |= 0x40; // end

			frame_data = (uint8_t *)malloc(remainPktSize + sizeof(RtpHdr) + 3);
			memset(frame_data, 0, remainPktSize + sizeof(RtpHdr) + 3);
			rtpPacket->m = 1;
			host_to_net(rtpPacket);
			memcpy(frame_data, rtpPacket, sizeof(RtpHdr));
			net_to_host(rtpPacket);
			rtp_ph = htons(rtp_ph);
			memcpy(frame_data + sizeof(RtpHdr), &rtp_ph, 2);
			memcpy(frame_data + sizeof(RtpHdr) + 2, &rtp_fu, 1);
			memcpy(frame_data + sizeof(RtpHdr) + 3, frame + pos, remainPktSize);
			ret = rtpSendPacket(sock, "video", frame_data, remainPktSize + 3 + sizeof(RtpHdr));
			if (ret < 0)
				return -1;

			rtpPacket->seq++;
			RtcpChar++;
			sendBytes += ret;
			
			if (frame_data)
			{
				free(frame_data);
				frame_data = NULL;
			}
		}
	}

out:
	return sendBytes;
}
/*查找特殊字符，分隔符 SEI等*/
bool MEDIA_STREAM::IsAccessSeparator(uint8_t *frame, int framesize, int &len, std::string mtype)
{
	int startCode = 0;
	char type = 0;
	int i = 0;

	if (framesize < 3)
		return false;

	if (startCode3(frame))
		startCode = 3;
	else
		startCode = 4;

	if (mtype == "h265")
		type = ((frame[startCode] & 0x7e) >> 1);
	if (mtype == "h264")
		type = (frame[startCode] & 0x1F);
	if (type == 35 || type == 9 || type == 6 || type == 39)
	{
		len = startCode;
		frame = frame + startCode;
		for (i = 0; i < len - 3; ++i)
		{
			if (startCode3(frame) || startCode4(frame))
				return true;
			++frame;
			++len;
		}

		if (startCode3(frame))
			return true;
	}

	return false;
}

/*函数描述：创建RTP发送线程并实现音视频同步*/
void MEDIA_STREAM::thread_proc(void)
{
	int frameSize = 0;
	int startCode = 0;
	RtpHdr v_rtpPacket;
	RtpHdr a_rtpPacket;
	RtcpHdr v_rtcpPacket;
	RtcpHdr a_rtcpPacket;
	int adtsSize = 0;
	int ret = 0;
	SOCK_T *m_sock = NULL;
	RTSP_S rtsp_s;
	std::string pth_name;
	uint64_t v_pts = 0;
	uint64_t a_pts = 0;
	std::string source_name;
	SOURCE *source_msg = NULL;


	int seqcount = 0;
	int RtpChNum = 0;
	int RtpPkNum = 0;
	uint64_t lastpts = 0;
	int i = 0;

	pth_name = get_pthread_name();

	memset((char *)&v_rtpPacket, 0, sizeof(v_rtpPacket));
	memset((char *)&a_rtpPacket, 0, sizeof(a_rtpPacket));

	std::list<CLI_MSG>::iterator client_msg_list;
	std::map<std::string, SOURCE>::iterator source_map;

	//根据rtsp地址路径名查找对应的源信息
	rtsp_s.source_mutex.mutex_lock();
	for (source_map = rtsp_s.rtsp_session.begin(); source_map != rtsp_s.rtsp_session.end(); source_map++)
	{
		if (source_map->first == pth_name)
		{
			source_name = source_map->first;
			source_msg = &source_map->second;
			break;
		}
	}
	rtsp_s.source_mutex.mutex_unlock();
	//初始化RTP头 RTCP头
	v_rtp_head_init(&v_rtpPacket, source_msg->ssrc);
	a_rtp_head_init(&a_rtpPacket, source_msg->ssrc);
	v_rtcp_head_init(&v_rtcpPacket, source_msg->ssrc);
	a_rtcp_head_init(&a_rtcpPacket, source_msg->ssrc);

	printf("start run play pthread! name = %s\n", source_name.c_str());

	while (IsDestroy() == false)
	{	
		if (!rtsp_s._f_list.empty())
		{
			//判断该源的客户端中是否有播放指令，没有的话不进行媒体就读取。
			source_msg->s_mutex.mutex_lock();
			for (client_msg_list = rtsp_s._f_list.begin(); client_msg_list != rtsp_s._f_list.end(); client_msg_list++)
			{
				if (client_msg_list->session_map->get_source_name() == pth_name)
				{
					if (client_msg_list->session_map->isPlayState())
					{
						source_msg->_play = true;
						break;
					}
				}
			}
			source_msg->s_mutex.mutex_unlock();
			
			//如果有客户端需要播放，进行推流
			if (source_msg->_play)
			{
				unsigned char *frame = (unsigned char *)malloc(100000);
				memset(frame, 0, 100000);
				int asnum = 0;
				unsigned char *asframe = frame;
				//根据时间戳进行音视频同步
			    if(a_pts >= v_pts){
					//读取音频帧
					frameSize = getFrameOfVideo(frame, pth_name, &v_pts);		
					if(frameSize < 0)
					{
						break;
					}
//					printf("video pts : %lld\n", v_ptsU);
					//去除分隔符 SEI等不必要的信息
					if(IsAccessSeparator(frame, frameSize, asnum, source_msg->video_format))
					{
						asframe = frame + asnum;
					}

					
					if(startCode3(asframe))
						startCode = 3;
					else
						startCode = 4;

					frameSize -= (startCode+asnum);

					//备份RTCP需要的信息
					v_rtpPacket.ts = v_pts;
					seqcount = v_rtpPacket.seq;
					RtpPkNum = source_msg->v_Rtcp_pNum;
					RtpChNum = source_msg->v_Rtcp_cNum;

                    source_msg->s_mutex.mutex_lock();
					for(client_msg_list = rtsp_s._f_list.begin(); client_msg_list != rtsp_s._f_list.end(); client_msg_list++)
					{

                        if(client_msg_list->session_map->get_source_name()== source_name)
                        {
							//刷新RTCP的信息，保证每个客户端的信息起始相同
							v_rtpPacket.seq = seqcount;
							source_msg->v_Rtcp_cNum = RtpChNum;
							source_msg->v_Rtcp_pNum = RtpPkNum;
					        if(client_msg_list->session_map->isPlayState())
							{
								//根据视频类型 进行填充并发送
								lastpts = v_rtpPacket.ts;
								m_sock = client_msg_list->session_map->get_socket();
								if(source_msg->video_format == "h264")
									source_msg->v_Rtcp_cNum += rtpSendH264Frame(m_sock, &v_rtpPacket, asframe+startCode, frameSize, source_msg->v_Rtcp_pNum);
								else if(source_msg->video_format == "h265")
									source_msg->v_Rtcp_cNum += rtpSendH265Frame(m_sock, &v_rtpPacket, asframe+startCode, frameSize, source_msg->v_Rtcp_pNum);
								rtcpSendControl(m_sock, "video", v_rtcpPacket, source_msg->v_Rtcp_pNum, v_pts, source_msg->v_Rtcp_cNum);
							}      
                        }
				    }
					source_msg->s_mutex.mutex_unlock();

				}
				if(a_pts < v_pts)
				{
					//获取音频帧
					frameSize = getFrameOfAudio(frame, pth_name, &a_pts);
					if(frameSize < 0)
					{
						break;
					}

//					printf("audio pts : %lld\n", a_pts);
					//获取ADTS头个数
					adtsSize = get_adts_bye(frame);
					frameSize -= adtsSize;

					a_rtpPacket.ts = a_pts;
					seqcount = a_rtpPacket.seq;
					RtpPkNum = source_msg->a_Rtcp_pNum;
					RtpChNum = source_msg->a_Rtcp_cNum;	

					source_msg->s_mutex.mutex_lock();
					for(client_msg_list = rtsp_s._f_list.begin(); client_msg_list != rtsp_s._f_list.end(); client_msg_list++)
					{

                        if(client_msg_list->session_map->get_source_name()==source_name)
                        {
							a_rtpPacket.seq = seqcount;
							source_msg->a_Rtcp_cNum = RtpChNum;
							source_msg->a_Rtcp_pNum = RtpPkNum;
					        if(client_msg_list->session_map->isPlayState())
							{
								//填充RTP并且发送 
								m_sock = client_msg_list->session_map->get_socket();                                
								source_msg->a_Rtcp_cNum += rtpSendAACFrame(m_sock, &a_rtpPacket, frame+adtsSize, frameSize, source_msg->a_Rtcp_pNum);
								rtcpSendControl(m_sock, "audio", a_rtcpPacket, source_msg->a_Rtcp_pNum, a_pts, source_msg->a_Rtcp_cNum);
							 }
                        }
					}
					source_msg->s_mutex.mutex_unlock();

				}
				if (frame)
				{
					free(frame);
					frame = NULL;
				}
			}
		}
		else{
			rtsp_s.source_mutex.mutex_lock();
			rtsp_s.source_mutex.mutex_cond_wait();
			rtsp_s.source_mutex.mutex_unlock();
		}
	}

	printf("free fram space !\n");

	return;
}


// change by yh 2022/1/17
/*判断三字节起始码*/
int MEDIA_STREAM::startCode3(unsigned char *buf)
{
	if (buf[0] == 0 && buf[1] == 0 && buf[2] == 1)
		return 1;
	else
		return 0;
}
/*判断四字节起始码*/
int MEDIA_STREAM::startCode4(unsigned char *buf)
{
	if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1)
		return 1;
	else
		return 0;
}

int MEDIA_STREAM::isSepChar(unsigned char *buf)
{
	if (buf[0] == 0x0 && buf[1] == 0x0 && buf[2] == 0x0 && buf[3] == 0x1)
	{
		if (buf[4] == 0x09)
			return 1;
		else
			return 0;
	}
}

uint8_t *MEDIA_STREAM::findNextStartCode(unsigned char *buf, int len)
{
	int i;

	if (len < 3)
		return NULL;

	for (i = 0; i < len - 3; ++i)
	{
		if (startCode3(buf) || startCode4(buf))
			return buf;

		++buf;
	}

	if (startCode3(buf))
		return buf;

	return NULL;
}

/*通过回调函数获取视频帧*/
int MEDIA_STREAM::getFrameOfVideo(unsigned char *frame, std::string media_source, uint64_t *v_pts)
{
	RTSP_S rtsp_server;
	int len = 0;
	int frameSize = 0;
	std::string source_name;
	SOURCE *source_msg = NULL;

	std::map<std::string, SOURCE>::iterator source_map;
	for (source_map = rtsp_server.rtsp_session.begin(); source_map != rtsp_server.rtsp_session.end(); source_map++)
	{
		source_name = source_map->first;
		source_msg = &source_map->second;
		if (source_name == media_source)
		{
			if (source_msg->read_video_callback != NULL)
			{
				frameSize = source_msg->read_video_callback(NULL, frame, len, v_pts);
			}
		}
	}

	return frameSize;
}
/*通过回调函数获取音频帧*/
int MEDIA_STREAM::getFrameOfAudio(unsigned char *frame, std::string media_source, uint64_t *a_pts)
{
	RTSP_S rtsp_server;
	int len = 0;
	int frameSize = 0;
	std::string source_name;
	SOURCE *source_msg = NULL;

	std::map<std::string, SOURCE>::iterator source_map;
	for (source_map = rtsp_server.rtsp_session.begin(); source_map != rtsp_server.rtsp_session.end(); source_map++)
	{
		source_name = source_map->first;
		source_msg = &source_map->second;
		if (source_name == media_source)
		{
			if (source_msg->read_audio_callback != NULL)
			{
				frameSize = source_msg->read_audio_callback(NULL, frame, len, a_pts);
			}
		}
	}

	return frameSize;
}
