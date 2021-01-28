//
//  main.cpp
//  ReverbTool
//
//  Created by amos-mbp on 2018/1/25.
//  Copyright © 2018年 amos-mbp. All rights reserved.
//

#include <iostream>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "AACDecoder.hpp"
#include "AACEncoder.hpp"
#include "Reverb.hpp"

#define DEFAULT_LENGTH 2048

//命令行参数
#define SEE
#define OPTION_DRY      'a'
#define OPTION_WET      'b'
#define OPTION_WIDTH    'c'
#define OPTION_ROOMSIZE 'd'
#define OPTION_MIX      'e'
#define OPTION_DAMP     'f'
#define OPTION_IN       'g'
#define OPTION_OUT      'h'

struct option long_option[] = {
    {"dry",      required_argument,0,OPTION_DRY},
    {"wet",      required_argument,0,OPTION_WET},
    {"width",    required_argument,0,OPTION_WIDTH},
    {"roomSize", required_argument,0,OPTION_ROOMSIZE},
    {"mix",      required_argument,0,OPTION_MIX},
    {"damp",     required_argument,0,OPTION_DAMP},
    {"in",       required_argument,0,OPTION_IN},
    {"out",      required_argument,0,OPTION_OUT},
    {0,0,0,0}
};

typedef struct WAV_HEADER_S
{
    char            riffType[4];    //4byte,资源交换文件标志:RIFF
    unsigned int    riffSize;       //4byte,从下个地址到文件结尾的总字节数
    char            waveType[4];    //4byte,wav文件标志:WAVE
    char            formatType[4];  //4byte,波形文件标志:FMT(最后一位空格符)
    unsigned int    formatSize;     //4byte,音频属性(compressionCode,numChannels,sampleRate,bytesPerSecond,blockAlign,bitsPerSample)所占字节数
    unsigned short  compressionCode;//2byte,格式种类(1-线性pcm-WAVE_FORMAT_PCM,WAVEFORMAT_ADPCM)
    unsigned short  numChannels;    //2byte,通道数
    unsigned int    sampleRate;     //4byte,采样率
    unsigned int    bytesPerSecond; //4byte,传输速率
    unsigned short  blockAlign;     //2byte,数据块的对齐，即DATA数据块长度
    unsigned short  bitsPerSample;  //2byte,采样精度-PCM位宽
    char            dataType[4];    //4byte,数据标志:data
    unsigned int    dataSize;       //4byte,从下个地址到文件结尾的总字节数，即除了wav header以外的pcm data length
}WAV_HEADER;

void creatWav(const char* pcm,const char* wav){
    
    char buffer[DEFAULT_LENGTH];
    FILE *pcmfile,*wavfile;
    
    pcmfile = fopen(pcm, "r");
    wavfile = fopen(wav, "wb+");
    
    fseek(pcmfile, 0, SEEK_END);
    
    unsigned int pcmLen = ftell(pcmfile);
    
    fseek(pcmfile, 0, SEEK_SET);
    
    WAV_HEADER header;
    
    header.riffType[0] = 'R';
    header.riffType[1] = 'I';
    header.riffType[2] = 'F';
    header.riffType[3] = 'F';
    
    header.riffSize = pcmLen+36;
    
    header.waveType[0] = 'W';
    header.waveType[1] = 'A';
    header.waveType[2] = 'V';
    header.waveType[3] = 'E';
    
    header.formatType[0] = 'f';
    header.formatType[1] = 'm';
    header.formatType[2] = 't';
    header.formatType[3] = ' ';
    
    header.formatSize = 16;
    
    header.compressionCode = 1;
    
    header.numChannels = 2;
    
    header.sampleRate = 44100;
    
    header.bitsPerSample = 16;
    
    header.blockAlign = header.numChannels * (header.bitsPerSample >> 3);;
    
    header.bytesPerSecond = header.bitsPerSample * header.blockAlign;
    
    header.dataType[0] = 'd';
    header.dataType[1] = 'a';
    header.dataType[2] = 't';
    header.dataType[3] = 'a';
    
    header.dataSize = pcmLen;
    
    fwrite(&header, 1, sizeof(WAV_HEADER), wavfile);
    
    int readlen = 0;
    while (readlen<pcmLen) {
        long valid = fread(buffer, 1, DEFAULT_LENGTH, pcmfile);
        
        if (valid<0) {
            break;
        }
        
        readlen+=valid;
        
        fwrite(buffer, 1, valid, wavfile);
    }
    
    
    fclose(pcmfile);
    fclose(wavfile);
}


int main(int argc, const char * argv[]) {
    
    //混音设置 （参数 6个）
    float dry = 50;
    float wet = 50;
    float width = 50;
    float roomSize = 50;
    float mix = 50;
    float damp = 50;
    
    char inFile[128] = ""; // 参数
    char outFile[128] = ""; // 参数
    
    //解码命令行参数
    //./a --dry 50 --wet 50 --width 50 --roomSize 50 --mix 50 --damp 50 --in 1.aac --out out.aac
    char op=0;
    
    while((op=getopt_long(argc,argv,"a:b:c:d:e:f:g:h:",long_option,NULL)) != -1)
    {
        switch(op)
        {
            case OPTION_DRY:
                sscanf(optarg,"%f",&dry);
                break;
            case OPTION_WET:
                sscanf(optarg,"%f",&wet);
                break;
            case OPTION_WIDTH:
                sscanf(optarg,"%f",&width);
                break;
            case OPTION_ROOMSIZE:
                sscanf(optarg,"%f",&roomSize);
                break;
            case OPTION_MIX:
                sscanf(optarg,"%f",&mix);
                break;
            case OPTION_DAMP:
                sscanf(optarg,"%f",&damp);
                break;
            case OPTION_IN:
                memset(inFile,0,128*sizeof(char));
                sscanf(optarg,"%s",inFile);
                break;
            case OPTION_OUT:
                memset(outFile,0,128*sizeof(char));
                sscanf(optarg,"%s",outFile);
                break;
        }
    }
    
    dry /= 100;
    wet /= 100;
    width /= 100;
    roomSize /= 100;
    mix /= 100;
    damp /= 100;
    
    printf("dry      : %f\n",dry);
    printf("wet      : %f\n",wet);
    printf("width    : %f\n",width);
    printf("roomsize : %f\n",roomSize);
    printf("mix      : %f\n",mix);
    printf("damp     : %f\n",damp);
    printf("in       : %s\n",inFile);
    printf("out      : %s\n",outFile);
    
    if(inFile[0]=='\0' || outFile[0]=='\0'){
        printf("incorrectly filePath\n");
        return 0;
    }
    
    printf("start decode!\n");
    
//    FILE *pcm;
//    pcm = fopen("in.pcm", "wb+");
    
    AAC_Decoder decoder;
    AAC_Encoder encoder;
    
    AACReader* aac_reader = 0;
    AACWriter* aac_writer = 0;
    AACFrameS16* aac_frame;
    
    decoder.aac_init();
    decoder.aacreader_open(&aac_reader, inFile);
    int sampleRate = decoder.aacreader_get_sample_rate(aac_reader);
    int channels = decoder.aacreader_get_channels(aac_reader);
    
    encoder.aacwriter_open(&aac_writer, outFile, sampleRate,
                           channels, 128000);
    
    Reverb reverb(sampleRate);
    reverb.applyReverbFX(dry,wet,width,roomSize,mix,damp);
    
    while (decoder.aacreader_read_s16(aac_reader, &aac_frame) == AACREADER_ERR_NOERR){
        
        //执行混响/////////
        reverb.process(aac_frame->samples,aac_frame->sample_count);
        //////////////////
        
//        fwrite(aac_frame->samples, sizeof(short), aac_frame->sample_count*2, pcm);
        
        encoder.aacwriter_write_s16(aac_writer, aac_frame);
        
        aac_frame_s16_free(&aac_frame);
    }
    
    decoder.aacreader_close(&aac_reader);
    encoder.aacwriter_close(&aac_writer);
    
    printf("encode over\n");
//    fclose(pcm);
//    creatWav("in.pcm", "in.wav");

    return 0;
}
