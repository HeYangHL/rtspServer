#ifndef _TS_DECODER_HPP_
#define _TS_DECODER_HPP_

#include <iostream>
#include <stdio.h>
#include <vector>
#include "Def.h"
#include "pes.hpp"
#include "pat.hpp"
#include "pmt.hpp"


class TsDecoder
{
public:
TsDecoder(){};
~TsDecoder(){};

typedef std::vector<uint8_t> PACK_T;
int decode(PACK_T i_pkt,PACK_T& o_frame,uint64_t& o_pts,std::string& o_codecType);
private:
    PAT_PACK pat_pack;
    PMT_PACK pmt_pack;
    PES_PACK pes_pack;
    unsigned short _pmt_pid = 0;
    unsigned short _video_pid = 0;
    unsigned short _audio_pid = 0;
    unsigned char _video_type = 0;
    unsigned char _audio_type = 0;
    uint32_t pat_flag = 0;
    uint32_t pmt_flag = 0;
    PACK_T media_frame;
};

#endif

