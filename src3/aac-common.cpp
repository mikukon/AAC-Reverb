#include "aac-rw.h"

#include <stdlib.h>

#define max(a,b)  (((a) > (b)) ? (a) : (b))
#define min(a,b)  (((a) < (b)) ? (a) : (b))

AACFrame* aac_frame_alloc(int samples, int channels)
{
	AACFrame* result = (AACFrame*)calloc(1, sizeof(AACFrame));
	result->sample_count = samples;
	result->samples[0] = (float*)calloc(samples * channels, sizeof(float));
	for (int i = 0; i < channels; ++i)
	{
		if (i > 0)
			result->samples[i] = result->samples[i - 1] + result->sample_count;
	}
	return result;
}

void aac_frame_free(AACFrame** aac_frame)
{
	if (aac_frame && *aac_frame)
	{
		if ((*aac_frame)->samples[0])
			free((*aac_frame)->samples[0]);
		free(*aac_frame);
		*aac_frame = 0;
	}
}


AACFrameS16* aac_frame_s16_alloc(int samples, int channels)
{
	AACFrameS16* result = (AACFrameS16*)calloc(1, sizeof(AACFrameS16));
	result->sample_count = samples;
	result->samples = (short*)calloc(samples * channels, sizeof(short));
	return result;
}

void aac_frame_s16_free(AACFrameS16** aac_frame)
{
	if (aac_frame && *aac_frame)
	{
		if ((*aac_frame)->samples)
			free((*aac_frame)->samples);
		free(*aac_frame);
		*aac_frame = 0;
	}
}


AACFrameS16* aac_frame_to_s16(AACFrame* aac_frame, int channels)
{
	AACFrameS16* result = aac_frame_s16_alloc(aac_frame->sample_count, channels);
	if (result)
	{
		for(int i = 0; i < aac_frame->sample_count; ++i)
		{
			for(int j = 0; j < channels; ++j)
			{
				result->samples[i * channels + j] = (short)min(max(aac_frame->samples[j][i] * 0x7fff, -0x8000), 0x7fff);
			}
		}
	}
	return result;
}

AACFrame* aac_frame_from_s16(AACFrameS16* aac_frame, int channels)
{
	AACFrame* result = aac_frame_alloc(aac_frame->sample_count, channels);
	if (result)
	{
		for(int i = 0; i < aac_frame->sample_count; ++i)
		{
			for(int j = 0; j < channels; ++j)
			{
				result->samples[j][i] = (float)min(max((double)aac_frame->samples[i * channels + j] / 0x7fff, -1.0), 1.0);
			}
		}
	}
	return result;
}
