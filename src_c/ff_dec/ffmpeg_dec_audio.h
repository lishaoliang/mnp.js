#ifndef __FFMPEG_DEC_AUDIO_H__
#define __FFMPEG_DEC_AUDIO_H__

#include "klb_type.h"

#include "em_frame_yuv_wav.h"
#include "libavcodec/avcodec.h"
#include "list/klb_list.h"

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct ffmpeg_dec_audio_t_ ffmpeg_dec_audio_t;


ffmpeg_dec_audio_t* ffmpeg_dec_audio_create(enum AVCodecID id /*=AV_CODEC_ID_AAC*/, int tracks, int sample_rate, int bits_per_sample);
void ffmpeg_dec_audio_destroy(ffmpeg_dec_audio_t* p_dec);


int ffmpeg_dec_audio_decode(ffmpeg_dec_audio_t* p_dec, uint8_t* p_data, int data_len, int64_t pts, klb_list_t* p_list);


#ifdef __cplusplus
}
#endif

#endif // __FFMPEG_DEC_AUDIO_H__
//end
