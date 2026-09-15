#ifndef _FIFO_BUFFER_HPP_
#define _FIFO_BUFFER_HPP_


#include <iostream>
#include <stdint.h>
#include <list>

#include "Pthread_Lock.hpp"

class CHN_FIFO{
public:
    CHN_FIFO(){
        len = 0;
        pts = 0;
        data = NULL;
    }
    ~CHN_FIFO(){};
    int len = 0;
    uint64_t pts = 0;
    int8_t *data = nullptr;
};
class ENC_FIFO{
public:
    ENC_FIFO(){
        len = 0;
        pts = 0;
        key = false;
        data = NULL;
    }
    ~ENC_FIFO(){};
    int len = 0;
    uint64_t pts = 0;
    bool key = false;
    int8_t *data = nullptr;
};

class List{
public:
    List(){
        pthVar.Init_Mutex_Cond(mutex, r_cond, w_cond);
        pthVar.Init_Mutex_Cond(enc_mutex, r_enc_cond, w_enc_cond);
    }
    ~List(){};
    int write(int8_t *buf, int len, uint64_t pts, int ms);
    int read(int8_t **buf, int *len, uint64_t *pts, int ms);
    int write_enc(int8_t *buf, int len, uint64_t pts, bool key, int ms);
    int read_enc(int8_t **buf, int *len, uint64_t *pts, bool *key, int ms);
    std::list<CHN_FIFO> Get_List(void){return _i_list;}
    int Clear_Fifo(void);
    int clean_enc_fifo(void);
    int clean_fifo(void);

private:
    std::list<CHN_FIFO> _i_list;
    std::list<ENC_FIFO> _e_list;
    PTH_LOCK pthVar;
    pthread_cond_t r_cond;
    pthread_cond_t w_cond;
    pthread_mutex_t mutex;

    pthread_cond_t r_enc_cond;
    pthread_cond_t w_enc_cond;
    pthread_mutex_t enc_mutex;
};


#endif