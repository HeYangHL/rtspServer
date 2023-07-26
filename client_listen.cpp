#include "client_listen.hpp"
#include "rtspserver.hpp"

void CLI_LIST::thread_proc(void)
{
    RTSP_S rtsp_s;

    std::list<CLI_MSG>::iterator iter;

    while (1)
    {
        if (!rtsp_s._f_list.empty())
        {
            for (iter = rtsp_s._f_list.begin(); iter != rtsp_s._f_list.end(); iter++)
            {
                struct tcp_info info;
                int len = sizeof(info);
                getsockopt(iter->cli_tcp_fd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
                if (info.tcpi_state == TCP_CLOSE_WAIT)
                {
                    printf("client fd = %d disconnect!\n", iter->cli_tcp_fd);
                    #if 0
                    if (iter->cli_tcp_fd)
                        close(iter->cli_tcp_fd);

                    if (iter->session_map != NULL)
                    {
                        delete iter->session_map;
                        iter->session_map = NULL;
                    }

                    rtsp_s._f_list.erase(iter);
                    iter--;
                    if (iter->session_pid)
                    {
                        pthread_cancel(iter->session_pid);
                    }
                    #endif
                }
            }
        }
        else{
            rtsp_s.source_mutex.mutex_lock();
            rtsp_s.source_mutex.mutex_cond_wait();
            rtsp_s.source_mutex.mutex_unlock();
        }
    }
    return;
}
void CLI_LIST::start_client_listen()
{
    printf("start client listen proc!\n");
    create_pth("Listen Client");
}
