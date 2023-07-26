#ifndef _THREAD_RTSP_HPP_
#define _THREAD_RTSP_HPP_

#include <iostream>
#include <stdio.h>
#include <pthread.h>


class THREAD_RTSP
{
public:
    THREAD_RTSP(){};
    ~THREAD_RTSP(){};
    int create_pth(std::string pth_name);
    static void *thread_fun(void *arg);
    virtual void thread_proc(void){
        std::cout << "start pthread proc !" << std:: endl;
        return ;
    }
    bool IsDestroy(){return pth_is_destroyed;}
    bool IsRunning(){return pth_is_running;}
    void set_Destroy(bool val){pth_is_destroyed = val;}
    void set_Run(bool val){pth_is_running = val;}
    pthread_t get_pthread_pid(void){return pid;}
    void set_pthrea_des_stat(bool val){pth_is_destroyed = val;}
    void set_pthrea_run_stat(bool val){pth_is_running = val;}
    std::string get_pthread_name(){return pthread_name;}

private:
    std::string pthread_name;
    pthread_t pid;
    bool pth_is_destroyed;
    bool pth_is_running;
};


#endif
