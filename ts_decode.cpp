#include "ts_decode.hpp"

std::list<Frame_t *> UDP_TS::_v_list;
std::list<Frame_t *> UDP_TS::_a_list;
std::list<Frame_t *> UDP_TS::_v_h264_list;
std::list<Frame_t *> UDP_TS::_a_h264_list;

void UDP_TS::create_udp(std::string ip, int port)
{
    if(port == 9701)
    {
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if(fd < 0)
        {
            perror("create socket error!");
            return;
        }
        bind_udp(ip, port);
    }
    else{
        fd1 = socket(AF_INET, SOCK_DGRAM, 0);
        if(fd < 0)
        {
            perror("create socket 2  error!");
            return;
        }
        bind_udp(ip, port);
    }
    return;
}
void UDP_TS::bind_udp(std::string ip, int port)
{
    int ret = 0;
    if(port == 9701)
    {
        myaddr.sin_family = AF_INET;
        myaddr.sin_port = htons(port);
        myaddr.sin_addr.s_addr = inet_addr(ip.c_str());

        ret = bind(fd,(struct sockaddr*)&myaddr,sizeof(myaddr));
        if(ret < 0)
            printf("prot : %d bind socket error!\n", port);

        create_pth("tsDecode");
    }
    else{
        myaddrc.sin_family = AF_INET;
        myaddrc.sin_port = htons(port);
        myaddrc.sin_addr.s_addr = inet_addr(ip.c_str());
        ret = bind(fd1,(struct sockaddr*)&myaddrc,sizeof(myaddrc));
        if(ret < 0)
            printf("port : %d bind socket error!\n", port);
        create_pth("tsDecodec");
    }


    return;
}

void UDP_TS::start_decode(void)
{
    
    create_udp("0.0.0.0", 9701);
    usleep(1000000);
    create_udp("0.0.0.0", 9700);

    return ;
}


int UDP_TS::write_video(unsigned char* data, int size, uint64_t pts, int64_t ms)
{
    int ret = 0;

    ts_mutex.mutex_lock();

//    printf("===>video fifo size : %d\n", _a_list.size());
    while (_v_list.size()>=32){
        if(ms>0){

 //           if(_DEBUG) printf("[%s: %d] W Twait...\n", __FUNCTION__, __LINE__);
            if(ETIMEDOUT == ts_mutex.mutex_cond_timedwait(ms)){
                ts_mutex.mutex_unlock();
//                if(_DEBUG) printf("[%s: %d] W Twait timeout!\n", __FUNCTION__, __LINE__);             
                return ret;
            }
 //           if(_DEBUG) printf("[%s: %d] W Twait Condition is met. \n", __FUNCTION__, __LINE__);
            break;
        }else if(ms<0){
 //           if(_DEBUG) printf("[%s: %d] W wait...\n", __FUNCTION__, __LINE__);
            ts_mutex.mutex_cond_wait();
//            if(_DEBUG) printf("[%s: %d] W Twait Condition is met.\n", __FUNCTION__, __LINE__);
            break;
        }else{
            ts_mutex.mutex_unlock();
            return ret;
        }
    }
  
    //如果是BufList_wr_Resume触发，则此条件会满足，返0
    if(_v_list.size()>=32){
        ts_mutex.mutex_unlock();
        return ret;
    }

 //   printf("this is write video : \n");
 //   print_data(data, 32);

    //加入list节点
    Frame_t* f = new Frame_t;
    f->frame = new unsigned char[size];
    f->size = size;
    f->pts = pts;

    if(data){
        memcpy(f->frame, data, size);
    }else{
        memset(f->frame, 0x00, size);
    }

    ret = size;

    _v_list.push_back(f);


    ts_mutex.mutex_cond_signal();
    ts_mutex.mutex_unlock();

    return ret;
}

int UDP_TS::write_audio(unsigned char* data, int size, uint64_t pts, int64_t ms)
{
    int ret = 0;

    ts_mutex.mutex_lock();

    
    while (_a_list.size()>=1024){
        if(ms>0){

//            if(_DEBUG) printf("[%s: %d] W Twait...\n", __FUNCTION__, __LINE__);
            if(ETIMEDOUT == ts_mutex.mutex_cond_timedwait(ms)){
                ts_mutex.mutex_unlock();
 //               if(_DEBUG) printf("[%s: %d] W Twait timeout!\n", __FUNCTION__, __LINE__);             
                return ret;
            }
//            if(_DEBUG) printf("[%s: %d] W Twait Condition is met. \n", __FUNCTION__, __LINE__);
            break;
        }else if(ms<0){
 //           if(_DEBUG) printf("[%s: %d] W wait...\n", __FUNCTION__, __LINE__);
            ts_mutex.mutex_cond_wait();
 //           if(_DEBUG) printf("[%s: %d] W Twait Condition is met.\n", __FUNCTION__, __LINE__);
            break;
        }else{
            ts_mutex.mutex_unlock();
            return ret;
        }
    }

    //如果是BufList_wr_Resume触发，则此条
    if(_a_list.size()>=1024){
        ts_mutex.mutex_unlock();
        return ret;
    }

    //加入list节点
    Frame_t* f = new Frame_t;
    f->frame = new unsigned char[size];
    f->size = size;
    f->pts = pts;

    if(data){
        memcpy(f->frame, data, size);
    }else{
        memset(f->frame, 0x00, size);
    }

    ret = size;

    _a_list.push_back(f);


    ts_mutex.mutex_cond_signal();
    ts_mutex.mutex_unlock();

    return ret;
}


int UDP_TS::write_h264_video(unsigned char* data, int size, uint64_t pts, int64_t ms)
{
    int ret = 0;

    h264_mutex.mutex_lock();

//    printf("===>video fifo size : %d\n", _a_list.size());
    while (_v_h264_list.size()>=32){
        if(ms>0){

 //           if(_DEBUG) printf("[%s: %d] W Twait...\n", __FUNCTION__, __LINE__);
            if(ETIMEDOUT == h264_mutex.mutex_cond_timedwait(ms)){
                h264_mutex.mutex_unlock();
//                if(_DEBUG) printf("[%s: %d] W Twait timeout!\n", __FUNCTION__, __LINE__);             
                return ret;
            }
 //           if(_DEBUG) printf("[%s: %d] W Twait Condition is met. \n", __FUNCTION__, __LINE__);
            break;
        }else if(ms<0){
 //           if(_DEBUG) printf("[%s: %d] W wait...\n", __FUNCTION__, __LINE__);
            h264_mutex.mutex_cond_wait();
//            if(_DEBUG) printf("[%s: %d] W Twait Condition is met.\n", __FUNCTION__, __LINE__);
            break;
        }else{
            h264_mutex.mutex_unlock();
            return ret;
        }
    }
   
    //如果是BufList_wr_Resume触发，则此条件会满足，返0
    if(_v_h264_list.size()>=32){
        h264_mutex.mutex_unlock();
        return ret;
    }
//    printf("this is H264 write video : \n");
//    print_data(data, 32);
    //加入list节点
    Frame_t* f = new Frame_t;
    f->frame = new unsigned char[size];
    f->size = size;
    f->pts = pts;

    if(data){
        memcpy(f->frame, data, size);
    }else{
        memset(f->frame, 0x00, size);
    }

    ret = size;

    _v_h264_list.push_back(f);

    h264_mutex.mutex_cond_signal();
    h264_mutex.mutex_unlock();

    return ret;
}

int UDP_TS::write_h264_audio(unsigned char* data, int size, uint64_t pts, int64_t ms)
{
    int ret = 0;

    h264_mutex.mutex_lock();

    
    while (_a_h264_list.size()>=1024){
        if(ms>0){

//            if(_DEBUG) printf("[%s: %d] W Twait...\n", __FUNCTION__, __LINE__);
            if(ETIMEDOUT == h264_mutex.mutex_cond_timedwait(ms)){
                h264_mutex.mutex_unlock();
 //               if(_DEBUG) printf("[%s: %d] W Twait timeout!\n", __FUNCTION__, __LINE__);             
                return ret;
            }
//            if(_DEBUG) printf("[%s: %d] W Twait Condition is met. \n", __FUNCTION__, __LINE__);
            break;
        }else if(ms<0){
 //           if(_DEBUG) printf("[%s: %d] W wait...\n", __FUNCTION__, __LINE__);
            h264_mutex.mutex_cond_wait();
 //           if(_DEBUG) printf("[%s: %d] W Twait Condition is met.\n", __FUNCTION__, __LINE__);
            break;
        }else{
            h264_mutex.mutex_unlock();
            return ret;
        }
    }

    //如果是BufList_wr_Resume触发，则此条
    if(_a_h264_list.size()>=1024){
        h264_mutex.mutex_unlock();
        return ret;
    }

    //加入list节点
    Frame_t* f = new Frame_t;
    f->frame = new unsigned char[size];
    f->size = size;
    f->pts = pts;

    if(data){
        memcpy(f->frame, data, size);
    }else{
        memset(f->frame, 0x00, size);
    }

    ret = size;

    _a_h264_list.push_back(f);


    h264_mutex.mutex_cond_signal();
    h264_mutex.mutex_unlock();

    return ret;
}

int UDP_TS::read_video(unsigned char* data,int* size, uint64_t* pts, int64_t ms)
{
    int ret = 0;

    ts_mutex.mutex_lock();
    while (_v_list.empty()) {
        if(ms>0){
 //           if(_DEBUG) printf("[%s: %d] R Twait...\n", __FUNCTION__, __LINE__);
            if(ETIMEDOUT == ts_mutex.mutex_cond_timedwait(ms)){
                ts_mutex.mutex_unlock();
 //               if(_DEBUG) printf("[%s: %d] R Twait timeout!\n", __FUNCTION__, __LINE__);  
                return ret;
            }
 //           if(_DEBUG) printf("[%s: %d] R Twait Condition is met.\n", __FUNCTION__, __LINE__);
            break;
        }else if(ms<0){
//            if(_DEBUG) printf("[%s: %d] R wait... \n", __FUNCTION__, __LINE__);
            ts_mutex.mutex_cond_wait();
//            if(_DEBUG) printf("[%s: %d] R wait Condition is met.\n", __FUNCTION__, __LINE__);
            break;
        }else{
            ts_mutex.mutex_unlock();
            return ret;
        }
    }

    //如果是BufList_rd_Resume触发，则此条件会满足，返回0
    if(_v_list.empty()){
        ts_mutex.mutex_unlock();
        return ret;
    }

    //取出节点数据
    Frame_t* f = _v_list.front();
    if(f && data){
        if(f->frame){
             memcpy(data, f->frame, f->size);
             delete [] f->frame;
             f->frame = NULL;
        }else{
            ret = -1;
        }
        if(size){
            ret = f->size;
            *size = f->size;
        }
        if(pts)*pts = f->pts;
        
        _v_list.pop_front();
        delete f;
        f = NULL;
    }
    
    ts_mutex.mutex_cond_signal();
    ts_mutex.mutex_unlock();

    return ret;
}

int UDP_TS::read_audio(unsigned char* data,int* size, uint64_t* pts, int64_t ms)
{
    int ret = 0;

    ts_mutex.mutex_lock();
    while (_a_list.empty()) {
        if(ms>0){
//            if(_DEBUG) printf("[%s: %d] R Twait...\n", __FUNCTION__, __LI
            if(ETIMEDOUT == ts_mutex.mutex_cond_timedwait(ms)){
                ts_mutex.mutex_unlock();
//                if(_DEBUG) printf("[%s: %d] R Twait timeout!\n", __FUNCTI
                return ret;
            }
//            if(_DEBUG) printf("[%s: %d] R Twait Condition is met.\n", __F
            break;
        }else if(ms<0){
//            if(_DEBUG) printf("[%s: %d] R wait... \n", __FUNCTION__, __LI
            ts_mutex.mutex_cond_wait();
//            if(_DEBUG) printf("[%s: %d] R wait Condition is met.\n", __FU
            break;
        }else{
            ts_mutex.mutex_unlock();
            return ret;
        }
    }

    //如果是BufList_rd_Resume触发，则此条件会满足，返回0
    if(_a_list.empty()){
        ts_mutex.mutex_unlock();
        return ret;
    }

    //取出节点数据
    Frame_t* f = _a_list.front();
    if(f && data){
        if(f->frame){
             memcpy(data, f->frame, f->size);
             delete [] f->frame;
             f->frame = NULL;
        }else{
            ret = -1;
        }
        if(size){
            ret = f->size;
            *size = f->size;
        }
        if(pts)*pts = f->pts;
        
        _a_list.pop_front();
        delete f;
        f = NULL;
    }
    
    ts_mutex.mutex_cond_signal();
    ts_mutex.mutex_unlock();

    return ret;
}

int UDP_TS::read_h264_video(unsigned char* data,int* size, uint64_t* pts, int64_t ms)
{
    int ret = 0;

    h264_mutex.mutex_lock();
    while (_v_h264_list.empty()) {
        if(ms>0){
 //           if(_DEBUG) printf("[%s: %d] R Twait...\n", __FUNCTION__, __LINE__);
            if(ETIMEDOUT == h264_mutex.mutex_cond_timedwait(ms)){
                h264_mutex.mutex_unlock();
 //               if(_DEBUG) printf("[%s: %d] R Twait timeout!\n", __FUNCTION__, __LINE__);  
                return ret;
            }
 //           if(_DEBUG) printf("[%s: %d] R Twait Condition is met.\n", __FUNCTION__, __LINE__);
            break;
        }else if(ms<0){
//            if(_DEBUG) printf("[%s: %d] R wait... \n", __FUNCTION__, __LINE__);
            h264_mutex.mutex_cond_wait();
//            if(_DEBUG) printf("[%s: %d] R wait Condition is met.\n", __FUNCTION__, __LINE__);
            break;
        }else{
            h264_mutex.mutex_unlock();
            return ret;
        }
    }

    //如果是BufList_rd_Resume触发，则此条件会满足，返回0
    if(_v_h264_list.empty()){
        h264_mutex.mutex_unlock();
        return ret;
    }

    //取出节点数据
    Frame_t* f = _v_h264_list.front();
    if(f && data){
        if(f->frame){
             memcpy(data, f->frame, f->size);
             delete [] f->frame;
             f->frame = NULL;
        }else{
            ret = -1;
        }
        if(size){
            ret = f->size;
            *size = f->size;
        }
        if(pts)*pts = f->pts;
        
        _v_h264_list.pop_front();
        delete f;
        f = NULL;
    }
    
    h264_mutex.mutex_cond_signal();
    h264_mutex.mutex_unlock();

    return ret;
}

int UDP_TS::read_h264_audio(unsigned char* data,int* size, uint64_t* pts, int64_t ms)
{
    int ret = 0;

    h264_mutex.mutex_lock();
    while (_a_h264_list.empty()) {
        if(ms>0){
//            if(_DEBUG) printf("[%s: %d] R Twait...\n", __FUNCTION__, __LI
            if(ETIMEDOUT == h264_mutex.mutex_cond_timedwait(ms)){
                h264_mutex.mutex_unlock();
//                if(_DEBUG) printf("[%s: %d] R Twait timeout!\n", __FUNCTI
                return ret;
            }
//            if(_DEBUG) printf("[%s: %d] R Twait Condition is met.\n", __F
            break;
        }else if(ms<0){
//            if(_DEBUG) printf("[%s: %d] R wait... \n", __FUNCTION__, __LI
            h264_mutex.mutex_cond_wait();
//            if(_DEBUG) printf("[%s: %d] R wait Condition is met.\n", __FU
            break;
        }else{
            h264_mutex.mutex_unlock();
            return ret;
        }
    }

    //如果是BufList_rd_Resume触发，则此条件会满足，返回0
    if(_a_h264_list.empty()){
        h264_mutex.mutex_unlock();
        return ret;
    }

    //取出节点数据
    Frame_t* f = _a_h264_list.front();
    if(f && data){
        if(f->frame){
             memcpy(data, f->frame, f->size);
             delete [] f->frame;
             f->frame = NULL;
        }else{
            ret = -1;
        }
        if(size){
            ret = f->size;
            *size = f->size;
        }
        if(pts)*pts = f->pts;
        
        _a_h264_list.pop_front();
        delete f;
        f = NULL;
    }
    
    h264_mutex.mutex_cond_signal();
    h264_mutex.mutex_unlock();

    return ret;
}

void UDP_TS::thread_proc(void)
{
    
    uint64_t pts = 0;
    std::string type;
    TsDecoder ts_de;
    TsDecoder::PACK_T ts_data;
    TsDecoder::PACK_T frame_data;
    struct sockaddr_in caddr;
    int ts_size = 0;
    int fdc = 0;
    std::string pname = get_pthread_name();
    if(pname == "tsDecode")
        fdc = fd;
    if(pname == "tsDecodec")
        fdc = fd1;    
    int addr_len = sizeof(struct sockaddr_in);
    printf("====>fdc = %d, fd = %d, fd1 = %d\n", fdc, fd, fd1);

    while(1)
    {
        fd_set rfd;
        fd_set efd;
        FD_ZERO(&rfd);
        FD_ZERO(&efd);
        FD_SET(fdc, &rfd);
        FD_SET(fdc, &efd);
        struct timeval tv = {0L, 1000L};
//        uint8_t *frame = new uint8_t[1024*100];
        uint8_t *ts = new uint8_t[1316];
        int ret = 0;
        int tscount = 0;


        ts_data.clear();

        memset(ts, 0, 188);
//        memset(frame, 0, 1024*100);
        ret = select(fdc+1, &rfd, NULL, &efd, &tv);
        if(ret > 0)
        {
            if(FD_ISSET(fdc, &rfd))
            {
//                recvfrom(fd, ts, 188, 0, (struct sockaddr*)&caddr, (socklen_t *)&addr_len);
//                memset(ts, 0, 1316);
                ts_size = read(fdc, ts, 1500);
                while(ts_size)
                {
                    ts_data.clear();

                    for(int i = 0; i < 188; i++)
                        ts_data.push_back(ts[tscount + i]);

 //                   printf("====>video ts data:\n");
 //                   print_data((unsigned char *)&ts[tscount], 32);
                    ts_size -= 188;
                    tscount += 188;
                    
                    

                    ret = ts_de.decode(ts_data, frame_data, pts, type);
                    if(ret == 0)
                        continue;
                    if(ret > 0)
                    {
//                        printf("media pts : %lld, type = %s, size = %d\n", pts, type.c_str(), ret);
                        if(type == "h264")
                        {
                            struct timeval tv;
                            gettimeofday(&tv,NULL);
                            // printf("microsecond:%ld\n",tv.tv_sec*1000000 + tv.tv_usec);
                            if(pname == "tsDecode")
                                write_video((unsigned char *)&frame_data[0], ret, pts, 10);
                            if(pname == "tsDecodec")
                                write_h264_video((unsigned char *)&frame_data[0], ret, pts, 10);
                        }
                        else if(type == "aac")
                        {
                            if(pname == "tsDecode")
                                write_audio((unsigned char *)&frame_data[0], ret, pts, 10);
                            if(pname == "tsDecodec")
                                write_h264_audio((unsigned char *)&frame_data[0], ret, pts, 10);
                        }
                         else if(type == "h265")
                        {
                            struct timeval tv;
                            gettimeofday(&tv,NULL);
                            // printf("microsecond:%ld\n",tv.tv_sec*1000000 + tv.tv_usec);
                            if(pname == "tsDecode")
                                write_video((unsigned char *)&frame_data[0], ret, pts, 10);
                            if(pname == "tsDecodec")
                                write_h264_video((unsigned char *)&frame_data[0], ret, pts, 10);
                        }
                    }
                }
            }
            if(FD_ISSET(fdc, &efd))
            {
                printf("select error!\n");
            }
        }
        if(ts)
        {
            delete [] ts;
            ts = NULL;
        }
    }
    
    
}