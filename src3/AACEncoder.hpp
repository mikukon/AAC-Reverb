//
//  AACEncoder.hpp
//  
//
//  Created by amos-mbp on 2018/1/25.
//
//

#ifndef AACEncoder_hpp
#define AACEncoder_hpp

#include <stdio.h>
#include <stdlib.h>
#include "aac-rw.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avstring.h>
#include <libavutil/audio_fifo.h>
}

typedef struct AACWriter
{
    AVFormatContext* format_context;
    AVStream* audio_stream;
    AVCodec* audio_encoder;
    AVCodecContext* audio_encoder_context;
    AVAudioFifo* audio_fifo;
    
    long long pts;
    
    int succeed;
} AACWriter;

class AAC_Encoder{
public:
    AAC_Encoder();
    ~AAC_Encoder();
    
public:
    enum aacwriter_err_t aacwriter_open(AACWriter** presult, const char* filename, int samplerate, int channels, int bitrate);
    enum aacwriter_err_t aacwriter_write(AACWriter* aac, AACFrame* frame);
    enum aacwriter_err_t aacwriter_write_s16(AACWriter* aac, AACFrameS16* frame);
    enum aacwriter_err_t aacwriter_close(AACWriter** paac);
    
private:
    enum aacwriter_err_t aacwriter_flush_input_buffer(AACWriter* aac, int eos);
    enum aacwriter_err_t aacwriter_feed(AACWriter* aac, AACFrame* frame);
    
};




#endif /* AACEncoder_hpp */
