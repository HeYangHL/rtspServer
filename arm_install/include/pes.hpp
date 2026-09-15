#ifndef _PES_HPP_
#define _PES_HPP_

#include "pat.hpp"

#pragma pack (1)
typedef struct PES_Header{
    unsigned packet_start_code_prefix   :24;//起始码
    unsigned stream_id                  :8;//流ID
    unsigned PES_packet_length          :16;//pes长度
    unsigned pes_flage                  :2;//固定10
    unsigned PES_scrambling_control     :2;
    unsigned PES_priority               :1;
    unsigned Data_alignment_indicator   :1;
    unsigned copyright                  :1;
    unsigned Original_or_copy           :1;
    unsigned flags                      :8;
    unsigned PES_head_len               :8;
    unsigned flags_data                 :40;
//    unsigned fill                       :8;//0xFF
}PES_Header;
#pragma pack ()
class PES_PACK
{
public:
    PES_PACK(){};
    ~PES_PACK(){};
    static int flag;
    void Set_Pes_Pack(unsigned char *es, uint64_t pts, int size, int type, uint8_t *pes_out, uint32_t *pes_out_len);
    void set_pts_dts(uint64_t us, uint8_t *buf);
//    TS_PACK *get_ts_pack();

    int parse_pes_head(unsigned char *ts_data);
    void parse_set_pes_pid(unsigned short video_pid, unsigned short audio_pid, unsigned short video_type, unsigned short audio_type)
    {
        _video_pid = video_pid;
        _audio_pid = audio_pid;
        _video_type = video_type;
        _audio_type = audio_type;
    }
    unsigned char get_media_flag(){return media_flag;}
    void set_media_flag(unsigned char m_flag){media_flag = m_flag;}
    unsigned char get_return_type(){return return_type;}
    uint64_t get_return_pts(){return return_pts;}
    std::vector<uint8_t> get_media_frame(){return media_frame;}
    void clear_media_frame(){media_frame.clear();}
    void get_pts_us(uint8_t *pts_val, uint64_t &pts);
private:
    PES_Header pes_pack_head;
    unsigned short _video_pid = 0;
    unsigned short _audio_pid = 0;
    unsigned char _video_type = 0;
    unsigned char _audio_type = 0;
    unsigned char media_flag = 0;
    unsigned char return_type = 0;
    unsigned char new_type = 0;
    uint64_t return_pts = 0;
    uint64_t new_pts = 0;
    std::vector<uint8_t> media_frame;
    std::vector<uint8_t> back_frame;
};

#endif