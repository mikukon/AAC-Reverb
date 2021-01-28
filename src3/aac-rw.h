#ifndef AAC_RW_H_
#define AAC_RW_H_

typedef struct AACFrame
{
	int sample_count;
	float* samples[8];
} AACFrame;

AACFrame* aac_frame_alloc(int samples, int channels);
void aac_frame_free(AACFrame** aac_frame);


typedef struct AACFrameS16
{
	int sample_count;
	short* samples;
} AACFrameS16;

AACFrameS16* aac_frame_s16_alloc(int samples, int channels);
void aac_frame_s16_free(AACFrameS16** aac_frame);


AACFrameS16* aac_frame_to_s16(AACFrame* aac_frame, int channels);
AACFrame* aac_frame_from_s16(AACFrameS16* aac_frame, int channels);


typedef struct AACReader AACReader;

enum aacreader_err_t {
	AACREADER_ERR_NOERR = 0,
	AACREADER_ERR_OPEN_FILE_FAILED,
	AACREADER_ERR_NO_AUDIO,
	AACREADER_ERR_NO_DECODER,
	AACREADER_ERR_FAIL_OPEN_DECODER,
	AACREADER_ERR_NOT_FLOAT_SAMPLEFMT,
	AACREADER_ERR_DECODE_ERROR,
	AACREADER_ERR_READ_PACKET_FAILED,
	AACREADER_ERR_FAIL_ALLOC_BUFFER,
	AACREADER_ERR_END_OF_STREAM
};

void aac_init();

enum aacreader_err_t aacreader_open(AACReader** presult, const char* filename);
enum aacreader_err_t aacreader_close(AACReader** aac);
enum aacreader_err_t aacreader_read(AACReader* aac, AACFrame** output);
enum aacreader_err_t aacreader_read_s16(AACReader* aac, AACFrameS16** output);
int aacreader_get_channels(AACReader* aac);
int aacreader_get_sample_rate(AACReader* aac);


typedef struct AACWriter AACWriter;

enum aacwriter_err_t {
	AACWRITER_ERR_NOERR = 0,
	AACWRITER_ERR_NO_ENCODER,
	AACWRITER_ERR_OPEN_ENCODER,
	AACWRITER_ERR_OPEN_FILE_FAILED,
	AACWRITER_ERR_NO_ADTS_MUXER,
	AACWRITER_ERR_FAIL_ADD_STREAM,
	AACWRITER_ERR_FAIL_SET_CODECPAR,
	AACWRITER_ERR_FAIL_ALLOC_BUFFER,
	AACWRITER_ERR_FAIL_READ_BUFFER,
	AACWRITER_ERR_FAIL_WRITE_HEADER,
	AACWRITER_ERR_FAIL_ENCODE,
	AACWRITER_ERR_FAIL_WRITE_TAILER
};

enum aacwriter_err_t aacwriter_open(AACWriter** presult, const char* filename, int samplerate, int channels, int bitrate);
enum aacwriter_err_t aacwriter_close(AACWriter** aac);
enum aacwriter_err_t aacwriter_write(AACWriter* aac, AACFrame* frame);
enum aacwriter_err_t aacwriter_write_s16(AACWriter* aac, AACFrameS16* frame);

#endif
