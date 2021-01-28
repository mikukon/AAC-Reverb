//
//  AACDecoder.cpp
//  ReverbTool
//
//  Created by amos-mbp on 2018/1/25.
//  Copyright © 2018年 amos-mbp. All rights reserved.
//

#include "AACDecoder.hpp"

AAC_Decoder::AAC_Decoder(){
}


AAC_Decoder::~AAC_Decoder(){
}


void AAC_Decoder::aac_init(){
    
    av_register_all();
    
}


enum aacreader_err_t AAC_Decoder::aacreader_open(AACReader** presult, const char* filename)
{
//    printf("1\n");

    enum aacreader_err_t error_code = AACREADER_ERR_NOERR;
    AACReader* result = (AACReader*) calloc(1, sizeof(AACReader));
//    printf("2\n");

    if (avformat_open_input(&result->format_context, filename, 0, 0) != 0)
    {
//        printf("3\n");

        error_code = AACREADER_ERR_OPEN_FILE_FAILED;
        goto cleanup;
    }
    
//    printf("4\n");

    if (avformat_find_stream_info(result->format_context, 0) < 0)
    {
//        printf("5\n");

        error_code = AACREADER_ERR_NO_AUDIO;
        goto cleanup;
    }
    
//    printf("6\n");

    result->audio_stream_number = av_find_best_stream(result->format_context, AVMEDIA_TYPE_AUDIO, -1, -1, &result->audio_decoder, 0);
    
//    printf("7\n");

    if (result->audio_stream_number < 0)
    {
//        printf("8\n");

        error_code = AACREADER_ERR_NO_AUDIO;
        goto cleanup;
    }
    
//    printf("9\n");
    if (result->audio_decoder == 0)
    {
//        printf("10\n");

        error_code = AACREADER_ERR_NO_DECODER;
        goto cleanup;
    }
    
//    printf("11\n");

    result->audio_decoder_context = avcodec_alloc_context3(result->audio_decoder);
    {
//        printf("12\n");

        AVStream* stream = result->format_context->streams[result->audio_stream_number];
        if (avcodec_parameters_to_context(result->audio_decoder_context, stream->codecpar) < 0)
        {
//            printf("13\n");

            error_code = AACREADER_ERR_FAIL_OPEN_DECODER;
            goto cleanup;
        }
        
//        printf("14\n");

        if (avcodec_open2(result->audio_decoder_context, result->audio_decoder, 0) < 0)
        {
//            printf("15\n");

            error_code = AACREADER_ERR_FAIL_OPEN_DECODER;
            goto cleanup;
        }
    }
    
//    printf("16\n");

    if (result->audio_decoder_context->sample_fmt != AV_SAMPLE_FMT_FLTP)
    {
//        printf("17\n");

        error_code = AACREADER_ERR_NOT_FLOAT_SAMPLEFMT;
        goto cleanup;
    }
    
//    printf("18\n");

    *presult = result;
    return AACREADER_ERR_NOERR;
    
cleanup:
    aacreader_close(&result);
    return error_code;
}


enum aacreader_err_t AAC_Decoder::aacreader_close(AACReader** aac){
    if (aac && *aac)
    {
        AACReader* result = *aac;
        
        avformat_close_input(&result->format_context);
        if (result->audio_decoder_context && avcodec_is_open(result->audio_decoder_context))
            avcodec_close(result->audio_decoder_context);
        avcodec_free_context(&result->audio_decoder_context);
        
        free(result);
        *aac = 0;
    }
    
    return AACREADER_ERR_NOERR;
}

int AAC_Decoder::aacreader_get_channels(AACReader* aac)
{
    return aac->audio_decoder_context->channels;
}

int AAC_Decoder::aacreader_get_sample_rate(AACReader* aac)
{
    return aac->audio_decoder_context->sample_rate;
}

enum aacreader_err_t AAC_Decoder::aacreader_read(AACReader* aac, AACFrame** output)
{
    AVPacket packet = { 0 };
    AVFrame* frame = 0;
    int ret;
    
get_result:
    frame = av_frame_alloc();
    ret = avcodec_receive_frame(aac->audio_decoder_context, frame);
    if (ret == 0)
    {
        AACFrame* result = aac_frame_alloc(frame->nb_samples, aac->audio_decoder_context->channels);
        for (int i = 0; i < aac->audio_decoder_context->channels; ++i)
        {
            memcpy(result->samples[i], frame->data[i], sizeof(float) * result->sample_count);
        }
        *output = result;
        av_frame_free(&frame);
        return AACREADER_ERR_NOERR;
    }
    
    av_frame_free(&frame);
    if (ret == AVERROR(EAGAIN))
    {
        av_frame_free(&frame);
        for (;;)
        {
            av_init_packet(&packet);
            if (av_read_frame(aac->format_context, &packet) != 0)
                return AACREADER_ERR_READ_PACKET_FAILED;
            if (packet.stream_index == aac->audio_stream_number)
                break;
            
            av_packet_unref(&packet);
        }
        
        ret = avcodec_send_packet(aac->audio_decoder_context, &packet);
        av_packet_unref(&packet);
        if (ret != 0)
            return AACREADER_ERR_DECODE_ERROR;
        
        goto get_result;
    }
    else if (ret == AVERROR_EOF)
    {
        return AACREADER_ERR_END_OF_STREAM;
    }
    else
        return AACREADER_ERR_DECODE_ERROR;
}


enum aacreader_err_t AAC_Decoder::aacreader_read_s16(AACReader* aac, AACFrameS16** output)
{
    AACFrame* tmp = 0;
    enum aacreader_err_t error_code = aacreader_read(aac, &tmp);
    if (error_code == AACREADER_ERR_NOERR)
    {
        AACFrameS16* result = aac_frame_to_s16(tmp, aacreader_get_channels(aac));
        aac_frame_free(&tmp);
        
        if (result == 0)
        {
            return AACREADER_ERR_FAIL_ALLOC_BUFFER;
        }
        else
        {
            *output = result;
            return AACREADER_ERR_NOERR;
        }
    }
    else
        return error_code;
}
