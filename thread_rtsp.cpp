#include "thread_rtsp.hpp"

void *THREAD_RTSP::thread_fun(void * arg)
{
    pthread_detach(pthread_self());
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);  
    
    THREAD_RTSP *thread = (THREAD_RTSP *)arg;
    thread->pth_is_destroyed = false;
    thread->pth_is_running = true;
    thread->thread_proc();
    thread->pth_is_destroyed = true;
    thread->pth_is_running = false;
    
    return NULL;
}

int THREAD_RTSP::create_pth(std::string pth_name)
{
    int ret = 0;

    pthread_name = pth_name;
    ret = pthread_create(&pid, NULL, thread_fun, this);
    if(ret < 0)
    {
        perror("create pthread error");
        return -1;
    }
    return 0;
}

