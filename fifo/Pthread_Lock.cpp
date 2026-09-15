#include "Pthread_Lock.hpp"

void PTH_LOCK::Destroy_sem(sem_t &sem)
{
    int ret = 0;

    ret = sem_destroy(&sem);
    if (ret < 0)
    {
        std::cout << "sem destroy error!" << std::endl;
        return;
    }

    return;
}

void PTH_LOCK::Init_Sem(sem_t &sem)
{
    int ret = 0;
    ret = sem_init(&sem, 0, 0);
    if (ret < 0)
    {
        std::cout << "sem init error !" << std::endl;
        return;
    }
    return;
}
void PTH_LOCK::Wait_Sem(sem_t &sem)
{
    int ret = 0;

    ret = sem_wait(&sem);
    if (ret < 0)
    {
        std::cout << "sem wait error!" << std::endl;
        return;
    }

    return;
}
void PTH_LOCK::Post_Sem(sem_t &sem)
{
    int ret = 0;

    ret = sem_post(&sem);
    if (ret < 0)
    {
        std::cout << "sem post error !" << std::endl;
        return;
    }

    return;
}

void PTH_LOCK::Init_Mutex_Cond(pthread_mutex_t &mutex, pthread_cond_t &r_cond, pthread_cond_t &w_cond)
{
    int ret = 0;

    ret = pthread_mutex_init(&mutex, NULL);
    if (ret < 0)
    {
        std::cout << "mutex init error!" << std::endl;
        return;
    }

    ret = pthread_cond_init(&r_cond, NULL);
    if (ret < 0)
    {
        std::cout << "cond init error!" << std::endl;
        return;
    }

    ret = pthread_cond_init(&w_cond, NULL);
    if (ret < 0)
    {
        std::cout << "cond init error!" << std::endl;
        return;
    }

    return;
}

void PTH_LOCK::Lock_Mutex(pthread_mutex_t &mutex)
{
    int ret = 0;

    ret = pthread_mutex_lock(&mutex);
    if (ret < 0)
    {
        std::cout << "mutex lock error!" << std::endl;
        return;
    }

    return;
}

void PTH_LOCK::Unlock_Mutex(pthread_mutex_t &mutex)
{
    int ret = 0;

    ret = pthread_mutex_unlock(&mutex);
    if (ret < 0)
    {
        std::cout << "mutex unlock error!" << std::endl;
        return;
    }

    return;
}

void PTH_LOCK::Destroy_Mutex_Cond(pthread_mutex_t &mutex, pthread_cond_t &r_cond, pthread_cond_t &w_cond)
{
    int ret = 0;

    ret = pthread_mutex_destroy(&mutex);
    if (ret < 0)
    {
        std::cout << "mutex destroy error!" << std::endl;
        return;
    }

    ret = pthread_cond_destroy(&r_cond);
    if (ret < 0)
    {
        std::cout << "mutex destroy error!" << std::endl;
        return;
    }

    ret = pthread_cond_destroy(&w_cond);
    if (ret < 0)
    {
        std::cout << "mutex destroy error!" << std::endl;
        return;
    }

    return;
}

int PTH_LOCK::Time_Wait_Cond(pthread_cond_t &cond, pthread_mutex_t &mutex, struct timespec time)
{
    int ret = 0;

    ret = pthread_cond_timedwait(&cond, &mutex, &time);
    if (ret < 0)
    {
        std::cout << "cond wait error!" << std::endl;
        return ret;
    }
    return ret;
}

void  PTH_LOCK::Wait_Cond(pthread_cond_t &cond, pthread_mutex_t &mutex)
{
    int ret = 0;
    ret = pthread_cond_wait(&cond, &mutex);
    if (ret < 0)
    {
        std::cout << "cond wait error!" << std::endl;
        return;
    }
    return;
}

void PTH_LOCK::Signal_Cond(pthread_cond_t &cond)
{
    int ret = 0;

    ret = pthread_cond_signal(&cond);
    if (ret < 0)
    {
        std::cout << "cond wait error!" << std::endl;
        return;
    }
    return;
}

int PTH_LOCK::Mutex_Init(void)
{
    int result = 0;
    result = pthread_mutex_init(&_mutex, NULL);
    return result;
}
int PTH_LOCK::Mutex_Lock(void)
{
    int ret = 0;
    ret = pthread_mutex_lock(&_mutex);
    return ret;
}
int PTH_LOCK::Mutex_Unlock(void)
{
    int ret = 0;
    ret = pthread_mutex_unlock(&_mutex);
    return ret;
}
int PTH_LOCK::Mutex_Destroy(void)
{
    int ret = 0;
    ret = pthread_mutex_destroy(&_mutex);
    return ret;
}