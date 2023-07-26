#include "ts_decoder.hpp"


int TsDecoder::decode(TsDecoder::PACK_T i_pkt,TsDecoder::PACK_T& o_frame,uint64_t& o_pts,std::string& o_codecType)
{
    unsigned int frame_len;
    unsigned char ts[188] = {0};
    std::vector<uint8_t> return_frame;
    int i = 0;
    int ret = 0;

    for(i = 0; i < i_pkt.size(); i++)
        ts[i] = i_pkt.at(i);
#if 0    
    ret = pat_pack.parse_pat(i_pkt, unsigned &_pmt_pid);
    if(ret == 0)
    {
        pmt_pack.parse_set_pmt_pid(_pmt_pid);
        ret = pmt_pack.parse_pmt(unsigned char * ts_data, &_video_pid, &_audio_pid, &_video_type, &_audio_type);
        if(ret == 0)
        {
            pes_pack.parse_set_pes_pid(_video_pid, _audio_pid);
            pes_pack.parse_pes_head(ts);
        }
    }
#endif
    if(pat_flag == 0)
    {        
        ret = pat_pack.parse_pat(ts, &_pmt_pid);
        if(ret == 0)
        {
            pat_flag = 1;
            return 0;
        }
    }
 //   printf("=======>pat_flag = %d\n", pat_flag);
    if(pat_flag == 1 && pmt_flag == 0)
    {       
        pmt_pack.parse_set_pmt_pid(_pmt_pid);


        ret = pmt_pack.parse_pmt(ts, &_video_pid, &_audio_pid, &_video_type, &_audio_type);
        if(ret == 0)
        {
//           printf("video pid = %02x, audio pid = %02x, type : %02x, %02x, pmt_pid = %02x\n", _video_pid, _audio_pid, _video_type,_audio_type, _pmt_pid);
            pmt_flag = 1;
            return 0;
        }
        
    }
    if(pat_flag == 1 && pmt_flag == 1)
    {
        pes_pack.parse_set_pes_pid(_video_pid, _audio_pid, _video_type, _audio_type);
        pes_pack.parse_pes_head(ts);
    }
    if(pes_pack.get_media_flag() == 2)
    {
        
        o_frame.clear();
        media_frame = pes_pack.get_media_frame();

        return_frame.insert(return_frame.end(), media_frame.begin(), media_frame.end());
        o_frame.insert(o_frame.end(), return_frame.begin(), return_frame.end());
        o_pts = pes_pack.get_return_pts();
        pes_pack.set_media_flag(1);
        if(pes_pack.get_return_type() == 0x1b)
            o_codecType = "h264";
        if(pes_pack.get_return_type() == 0x24)
            o_codecType = "h265";
        if(pes_pack.get_return_type() == 0x0f)
            o_codecType = "aac";

        
        pes_pack.clear_media_frame();

//        printf("====>type = %s, size = %d\n", o_codecType.c_str(), return_frame.size());
#if 0
        for(int i = 0; i < 32; i++)
        {
            if(i%16 == 0)
                printf("\n");
            printf("%02x ", o_frame.at(i));
        }
        printf("\n");
#endif
        return return_frame.size();
    }

    return 0;
}

