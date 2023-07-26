#ifndef _MUTEX_HPP_
#define _MUTEX_HPP_

#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <sys/time.h>


class MUTEX{
public:
    MUTEX();
    ~MUTEX();
    void mutex_lock(void);
    void mutex_unlock(void);
    void mutex_cond_wait(void);
    void mutex_cond_signal(void);
    int mutex_cond_timedwait(int64_t ms);
    void mutex_cond_broadcast(void);

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};


#endif
