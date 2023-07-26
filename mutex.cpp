#include "mutex.hpp"


MUTEX::MUTEX()
{
    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_cond, NULL);
}
MUTEX::~MUTEX()
{
    pthread_mutex_destroy(&m_mutex);
}

void MUTEX::mutex_lock(void)
{
    pthread_mutex_lock(&m_mutex);
}

void MUTEX::mutex_unlock(void)
{
    pthread_mutex_unlock(&m_mutex);
}
void MUTEX::mutex_cond_wait(void)
{
    pthread_cond_wait(&m_cond, &m_mutex);
}
void MUTEX::mutex_cond_signal(void)
{
    pthread_cond_signal(&m_cond);
}
int MUTEX::mutex_cond_timedwait(int64_t ms)
{
    struct timespec abstime;
    struct timeval now;
    gettimeofday(&now, NULL);
    long nsec = now.tv_usec * 1000 + (ms % 1000) * 1000000;
    abstime.tv_sec = now.tv_sec + nsec / 1000000000 + ms / 1000;
    abstime.tv_nsec = nsec % 1000000000;

    return pthread_cond_timedwait(&m_cond, &m_mutex, &abstime);
}

void MUTEX::mutex_cond_broadcast(void)
{
    pthread_cond_broadcast(&m_cond);
    
    return;
}