///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    ffmpeg_dec.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __FFMPEG_DEC_H__
#define __FFMPEG_DEC_H__

#include "klb_type.h"

#include "em_util/em_yuv_frame.h"
#include "libavcodec/avcodec.h"
#include "libavutil/frame.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct ffmpeg_dec_t_ ffmpeg_dec_t;

void ffmpeg_dec_init();

ffmpeg_dec_t* ffmpeg_dec_create(enum AVCodecID id /*=AV_CODEC_ID_H264*/);
void ffmpeg_dec_destroy(ffmpeg_dec_t* p_dec);

AVFrame* ffmpeg_dec_decode(ffmpeg_dec_t* p_dec, uint8_t* p_data, int data_len, int64_t pts, int64_t dts);

void ffmpeg_dec_av_frame_free(AVFrame* p_frame);


em_yuv_frame_t* ffmpeg_dec_decode2(ffmpeg_dec_t* p_dec, uint8_t* p_data, int data_len, int64_t pts, int64_t dts);
void ffmpeg_dec_yuv_frame_free(em_yuv_frame_t* p_yuv);


#ifdef __cplusplus
}
#endif

#endif // __FFMPEG_DEC_H__
//end
