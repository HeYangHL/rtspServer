#ifndef _PTHREAD_LOCK_HPP_
#define _PTHREAD_LOCK_HPP_

#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

class PTH_LOCK
{
public:
    PTH_LOCK(){
        Mutex_Init();
    };
    ~PTH_LOCK(){
        Mutex_Destroy();
    };
    void Init_Sem(sem_t &sem);
    void Wait_Sem(sem_t &sem);
    void Post_Sem(sem_t &sem);
    void Destroy_sem(sem_t &sem);
    void Init_Mutex_Cond(pthread_mutex_t &mutex, pthread_cond_t &r_cond, pthread_cond_t &w_cond);
    void Lock_Mutex(pthread_mutex_t &mutex);
    void Unlock_Mutex(pthread_mutex_t &mutex);
    void Destroy_Mutex_Cond(pthread_mutex_t &mutex, pthread_cond_t &r_cond, pthread_cond_t &w_cond);
    int Time_Wait_Cond(pthread_cond_t &cond, pthread_mutex_t &mutex, struct timespec time);
    void Wait_Cond(pthread_cond_t &cond, pthread_mutex_t &mutex);
    void Signal_Cond(pthread_cond_t &cond);

    int write_fifo(int8_t *buf, int len, uint64_t id, int ms);
    int read_fifo(int8_t **buf, int *len, uint64_t id, int ms);

    int Mutex_Init(void);
    int Mutex_Lock(void);
    int Mutex_Unlock(void);
    int Mutex_Destroy(void);

private:
    pthread_mutex_t _mutex;
};

#endif