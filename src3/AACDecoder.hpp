//
//  AACDecoder.hpp
//  ReverbTool
//
//  Created by amos-mbp on 2018/1/25.
//  Copyright © 2018年 amos-mbp. All rights reserved.
//

#ifndef AACDecoder_hpp
#define AACDecoder_hpp

#include <stdio.h>
#include <stdlib.h>
#include "aac-rw.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
//
}

typedef struct AACReader
{
    AVFormatContext* format_context;
    AVCodec* audio_decoder;
    AVCodecContext* audio_decoder_context;
    int audio_stream_number;
} AACReader;


class AAC_Decoder{
    
public:
    AAC_Decoder();
    ~AAC_Decoder();
    
public:
    void aac_init();
    enum aacreader_err_t aacreader_open(AACReader** presult, const char* filename);
    enum aacreader_err_t aacreader_close(AACReader** aac);
    int aacreader_get_channels(AACReader* aac);
    int aacreader_get_sample_rate(AACReader* aac);
    enum aacreader_err_t aacreader_read(AACReader* aac, AACFrame** output);
    enum aacreader_err_t aacreader_read_s16(AACReader* aac, AACFrameS16** output);
};

#endif /* AACDecoder_hpp */


