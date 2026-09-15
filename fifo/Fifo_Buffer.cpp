#include "Fifo_Buffer.hpp"

int List::Clear_Fifo(void)
{
    for (auto item = _i_list.begin(); item != _i_list.end(); item++)
    {
        // if (item->data != NULL)
        // {
        //     delete item->data;
        //     item->data = nullptr;
        // }
        if (item->data != NULL)
        {
            delete [] item->data;
            item->data = nullptr;
        }
    }

    return 0;
}

int List::write(int8_t *buf, int len, uint64_t pts, int ms)
{
    pthVar.Lock_Mutex(mutex);

    while (_i_list.size() > 31)
    {
        if (ms > 0)
        {
            struct timespec abstime;
            struct timeval now;
            gettimeofday(&now, NULL);
            long nsec = now.tv_usec * 1000 + (ms % 1000) * 1000000;
            abstime.tv_sec = now.tv_sec + nsec / 1000000000 + ms / 1000;
            abstime.tv_nsec = nsec % 1000000000;
            if (ETIMEDOUT == pthVar.Time_Wait_Cond(r_cond, mutex, abstime))
            {
                pthVar.Unlock_Mutex(mutex);
                return -1;
            }
            break;
        }
        if (ms < 0)
        {
            pthVar.Wait_Cond(r_cond, mutex);
            break;
        }
        else
        {
            pthVar.Unlock_Mutex(mutex);
            return 0;
        }
    }
    while (_i_list.size() > 31)
    {
        pthVar.Unlock_Mutex(mutex);
        return 0;
    }
    if(len <= 0)
        return 0;

    CHN_FIFO _fifo;
    _fifo.data = buf;
    _fifo.pts = pts;
    _fifo.len = len;

    _i_list.push_back(_fifo);

    pthVar.Signal_Cond(w_cond);
    pthVar.Unlock_Mutex(mutex);

    return len;
}

int List::read(int8_t **buf, int *len, uint64_t *pts, int ms)
{
    pthVar.Lock_Mutex(mutex);

    while (_i_list.empty())
    {
        if (ms > 0)
        {
            struct timespec abstime;
            struct timeval now;
            gettimeofday(&now, NULL);
            long nsec = now.tv_usec * 1000 + (ms % 1000) * 1000000;
            abstime.tv_sec = now.tv_sec + nsec / 1000000000 + ms / 1000;
            abstime.tv_nsec = nsec % 1000000000;
            if (ETIMEDOUT == pthVar.Time_Wait_Cond(w_cond, mutex, abstime))
            {
                pthVar.Unlock_Mutex(mutex);
                return -1;
            }
            break;
        }
        if (ms < 0)
        {
            pthVar.Wait_Cond(w_cond, mutex);
            break;
        }
        else
        {
            pthVar.Unlock_Mutex(mutex);
            return 0;
        }
    }
    while (_i_list.empty())
    {
        pthVar.Unlock_Mutex(mutex);
        return 0;
    }

    CHN_FIFO _fifo = _i_list.front();
    *buf = _fifo.data;
    *pts = _fifo.pts;
    *len = _fifo.len;

    _i_list.pop_front();

    pthVar.Signal_Cond(r_cond);
    pthVar.Unlock_Mutex(mutex);

    return _fifo.len;
}

int List::write_enc(int8_t *buf, int len, uint64_t pts, bool key, int ms)
{
    pthVar.Lock_Mutex(enc_mutex);
    while (_e_list.size() > 31)
    {
        if (ms > 0)
        {
            struct timespec abstime;
            struct timeval now;
            gettimeofday(&now, NULL);
            long nsec = now.tv_usec * 1000 + (ms % 1000) * 1000000;
            abstime.tv_sec = now.tv_sec + nsec / 1000000000 + ms / 1000;
            abstime.tv_nsec = nsec % 1000000000;
            if (ETIMEDOUT == pthVar.Time_Wait_Cond(r_enc_cond, enc_mutex, abstime))
            {
                pthVar.Unlock_Mutex(enc_mutex);
                return -1;
            }
            break;
        }
        if (ms < 0)
        {
            pthVar.Wait_Cond(r_enc_cond, enc_mutex);
            break;
        }
        else
        {
            pthVar.Unlock_Mutex(enc_mutex);
            return 0;
        }
    }
    while (_e_list.size() > 31)
    {
        pthVar.Unlock_Mutex(enc_mutex);
        return 0;
    }
    if(len <= 0)
        return 0;

    ENC_FIFO _fifo;
    _fifo.data = buf;
    _fifo.pts = pts;
    _fifo.key = key;
    _fifo.len = len;

    _e_list.push_back(_fifo);

    pthVar.Signal_Cond(w_enc_cond);
    pthVar.Unlock_Mutex(enc_mutex);
    return len;
}

int List::read_enc(int8_t **buf, int *len, uint64_t *pts, bool *key, int ms)
{
    printf("333\n");
    pthVar.Lock_Mutex(enc_mutex);

    printf("111\n");
    while (_e_list.empty())
    {
        printf("222\n");
        if (ms > 0)
        {
            struct timespec abstime;
            struct timeval now;
            gettimeofday(&now, NULL);
            long nsec = now.tv_usec * 1000 + (ms % 1000) * 1000000;
            abstime.tv_sec = now.tv_sec + nsec / 1000000000 + ms / 1000;
            abstime.tv_nsec = nsec % 1000000000;
            if (ETIMEDOUT == pthVar.Time_Wait_Cond(w_enc_cond, enc_mutex, abstime))
            {
                pthVar.Unlock_Mutex(enc_mutex);
                return -1;
            }
            break;
        }
        if (ms < 0)
        {
            pthVar.Wait_Cond(w_enc_cond, enc_mutex);
            break;
        }
        else
        {
            pthVar.Unlock_Mutex(enc_mutex);
            return 0;
        }
    }
    printf("333\n");
    while (_e_list.empty())
    {
        pthVar.Unlock_Mutex(enc_mutex);
        return 0;
    }
    printf("444\n");
    ENC_FIFO _fifo = _e_list.front();
    *buf = _fifo.data;
    *pts = _fifo.pts;
    *key = _fifo.key;
    *len = _fifo.len;
    _e_list.pop_front();
    printf("555 len : %d\n", _fifo.len);
    pthVar.Signal_Cond(r_enc_cond);
    pthVar.Unlock_Mutex(enc_mutex);
    printf("666\n");
    return _fifo.len;
}

int List::clean_enc_fifo(void)
{
    pthVar.Lock_Mutex(enc_mutex);

    while (!_e_list.empty())
    {
        auto &front = _e_list.front();
        if (front.data != nullptr)
        {
            delete[] front.data;
            front.data = nullptr;
        }
        _e_list.pop_front();
    }
#if 0
    for (auto item = _e_list.begin(); item != _e_list.end();)
    {
        if (item->data != nullptr)
        {
            delete[] item->data;
            item->data = nullptr;
        }
        item = _e_list.erase(item);
    }
#endif
    pthVar.Unlock_Mutex(enc_mutex);
}

int List::clean_fifo(void)
{
    pthVar.Lock_Mutex(mutex);

    while (!_i_list.empty())
    {
        auto &front = _e_list.front();
        if (front.data != nullptr)
        {
            delete[] front.data;
            front.data = nullptr;
        }
        _e_list.pop_front();
    }
#if 0
    for (auto item = _e_list.begin(); item != _e_list.end();)
    {
        if (item->data != nullptr)
        {
            delete[] item->data;
            item->data = nullptr;
        }
        item = _e_list.erase(item);
    }
#endif
    pthVar.Unlock_Mutex(mutex);
}