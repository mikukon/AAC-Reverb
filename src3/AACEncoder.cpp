//
//  AACEncoder.cpp
//  
//
//  Created by amos-mbp on 2018/1/25.
//
//
#include "AACEncoder.hpp"

AAC_Encoder::AAC_Encoder(){
    
}

AAC_Encoder::~AAC_Encoder(){
    
}

enum aacwriter_err_t AAC_Encoder::aacwriter_open(AACWriter** presult, const char* filename, int samplerate, int channels, int bitrate)
{
    enum aacwriter_err_t error_code = AACWRITER_ERR_NOERR;
    AACWriter* result;
    AVIOContext* avio_context;
    
    result = (AACWriter*)calloc(1, sizeof(AACWriter));
    result->format_context = avformat_alloc_context();
    
    result->audio_encoder = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (result->audio_encoder == 0)
    {
        error_code = AACWRITER_ERR_NO_ENCODER;
        goto cleanup;
    }
    
    if (avio_open(&avio_context, filename, AVIO_FLAG_WRITE) < 0)
    {
        error_code = AACWRITER_ERR_OPEN_FILE_FAILED;
        goto cleanup;
    }
    result->format_context->pb = avio_context;
    
    result->format_context->oformat = av_guess_format("adts", 0, 0);
    if (result->format_context->oformat == 0)
    {
        error_code = AACWRITER_ERR_NO_ADTS_MUXER;
        goto cleanup;
    }
    
    av_strlcpy(result->format_context->filename, filename, sizeof(result->format_context->filename));
    
    result->audio_stream = avformat_new_stream(result->format_context, result->audio_encoder);
    if (!result->audio_stream)
    {
        error_code = AACWRITER_ERR_FAIL_ADD_STREAM;
        goto cleanup;
    }
    
    result->audio_stream->time_base.num = 1;
    result->audio_stream->time_base.den = samplerate;
    
    result->audio_encoder_context = avcodec_alloc_context3(result->audio_encoder);
    if (result->audio_encoder_context == 0)
    {
        error_code = AACWRITER_ERR_OPEN_ENCODER;
        goto cleanup;
    }
    
    result->audio_encoder_context->channels = channels;
    result->audio_encoder_context->channel_layout = av_get_default_channel_layout(channels);
    result->audio_encoder_context->sample_rate = samplerate;
    result->audio_encoder_context->sample_fmt = AV_SAMPLE_FMT_FLTP;
    result->audio_encoder_context->bit_rate = bitrate;
    
    if (result->format_context->oformat->flags & AVFMT_GLOBALHEADER)
        result->audio_encoder_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        
        if (avcodec_open2(result->audio_encoder_context, result->audio_encoder, 0) < 0)
        {
            error_code = AACWRITER_ERR_OPEN_ENCODER;
            goto cleanup;
        }
    
    if (avcodec_parameters_from_context(result->audio_stream->codecpar, result->audio_encoder_context) < 0)
    {
        error_code = AACWRITER_ERR_FAIL_SET_CODECPAR;
        goto cleanup;
    }
    
    result->audio_fifo = av_audio_fifo_alloc(result->audio_encoder_context->sample_fmt, channels, 1);
    if (result->audio_fifo == 0)
    {
        error_code = AACWRITER_ERR_FAIL_ALLOC_BUFFER;
        goto cleanup;
    }
    
    if (avformat_write_header(result->format_context, 0) < 0)
    {
        error_code = AACWRITER_ERR_FAIL_WRITE_HEADER;
        goto cleanup;
    }
    
    result->succeed = 1;
    *presult = result;
    return AACWRITER_ERR_NOERR;
    
cleanup:
    aacwriter_close(&result);
    return error_code;
}

enum aacwriter_err_t AAC_Encoder::aacwriter_flush_input_buffer(AACWriter* aac, int eos)
{
    int ret;
    
    int frame_size = aac->audio_encoder_context->frame_size;
    while (eos || av_audio_fifo_size(aac->audio_fifo) >= frame_size)
    {
        AVFrame* frame = 0;
        
        if (!eos || (eos && av_audio_fifo_size(aac->audio_fifo) > 0))
        {
            frame_size = FFMIN(frame_size, av_audio_fifo_size(aac->audio_fifo));
            
            frame = av_frame_alloc();
            frame->pts = aac->pts;
            frame->nb_samples = frame_size;
            frame->channel_layout = aac->audio_encoder_context->channel_layout;
            frame->format = aac->audio_encoder_context->sample_fmt;
            frame->sample_rate = aac->audio_encoder_context->sample_rate;
            
            if (av_frame_get_buffer(frame, 0) < 0)
            {
                av_frame_free(&frame);
                return AACWRITER_ERR_FAIL_ALLOC_BUFFER;
            }
            
            //注意下这边-----------------------------------------------
            int readed_frame_size = av_audio_fifo_read(aac->audio_fifo, (void**)frame->data, frame_size);
            if (!eos && readed_frame_size < frame_size)
            {
                av_frame_free(&frame);
                return AACWRITER_ERR_FAIL_READ_BUFFER;
            }
        }
        
        if (avcodec_send_frame(aac->audio_encoder_context, frame) < 0)
        {
            av_frame_free(&frame);
            return AACWRITER_ERR_FAIL_ENCODE;
        }
        
        if (frame)
        {
            aac->pts += frame->nb_samples;
            av_frame_free(&frame);
        }
        
        for (;;)
        {
            AVPacket packet;
            av_init_packet(&packet);
            ret = avcodec_receive_packet(aac->audio_encoder_context, &packet);
            if (ret == 0)
            {
                av_write_frame(aac->format_context, &packet);
            }
            else if (ret == AVERROR(EAGAIN))
            {
                break;
            }
            else if (ret == AVERROR_EOF && eos)
            {
                eos = 0;
                break;
            }
            else
            {
                av_packet_unref(&packet);
                return AACWRITER_ERR_FAIL_ENCODE;
            }
            
            av_packet_unref(&packet);
        }
    }
    
    return AACWRITER_ERR_NOERR;
}

enum aacwriter_err_t AAC_Encoder::aacwriter_feed(AACWriter* aac, AACFrame* frame)
{
    if (av_audio_fifo_realloc(aac->audio_fifo, av_audio_fifo_size(aac->audio_fifo) + frame->sample_count) < 0)
        return AACWRITER_ERR_FAIL_ALLOC_BUFFER;
    
    //注意下这边----------------------------------------------------------
    if (av_audio_fifo_write(aac->audio_fifo, (void**)frame->samples, frame->sample_count) < frame->sample_count)
        return AACWRITER_ERR_FAIL_ALLOC_BUFFER;
    
    return AACWRITER_ERR_NOERR;
}

enum aacwriter_err_t AAC_Encoder::aacwriter_write(AACWriter* aac, AACFrame* frame)
{
    enum aacwriter_err_t ret;
    ret = aacwriter_feed(aac, frame);
    if (ret != AACWRITER_ERR_NOERR)
        return ret;
    ret = aacwriter_flush_input_buffer(aac, 0);
    if (ret != AACWRITER_ERR_NOERR)
        return ret;
    
    return AACWRITER_ERR_NOERR;
}

enum aacwriter_err_t AAC_Encoder::aacwriter_write_s16(AACWriter* aac, AACFrameS16* frame)
{
    AACFrame* tmp = aac_frame_from_s16(frame, aac->audio_encoder_context->channels);
    if (tmp == 0)
        return AACWRITER_ERR_FAIL_ALLOC_BUFFER;
    
    enum aacwriter_err_t error_code = aacwriter_write(aac, tmp);
    aac_frame_free(&tmp);
    return error_code;
}

enum aacwriter_err_t AAC_Encoder::aacwriter_close(AACWriter** paac)
{
    enum aacwriter_err_t error_code = AACWRITER_ERR_NOERR;
    if (paac && *paac)
    {
        AACWriter* aac = *paac;
        
        if (aac->succeed)
        {
            aacwriter_flush_input_buffer(aac, 1);
            if (av_write_trailer(aac->format_context) != 0)
            {
                error_code = AACWRITER_ERR_FAIL_WRITE_TAILER;
            }
        }
        
        if (aac->audio_fifo)
        {
            av_audio_fifo_free(aac->audio_fifo);
            aac->audio_fifo = 0;
        }
        
        if (aac->audio_encoder_context)
        {
            if (avcodec_is_open(aac->audio_encoder_context))
                avcodec_close(aac->audio_encoder_context);
                avcodec_free_context(&aac->audio_encoder_context);
                }
        
        if (aac->format_context)
        {
            avformat_free_context(aac->format_context);
            aac->format_context = 0;
        }
        
        free(aac);
        
        *paac = 0;
    }
    
    return AACWRITER_ERR_NOERR;
}
