///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    ff_dec_single.h
/// @brief   一路ffmpeg解码管理
///  \n 负责音视频解码, 负责音视频同步, 负责音视频帧率控制, 负责丢帧策略
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __FF_DEC_SINGLE_H__
#define __FF_DEC_SINGLE_H__

#include "klb_type.h"
#include "list/klb_list.h"
#include "thread/klb_thread.h"
#include "em_frame_yuv_wav.h"
#include "em_buf.h"
#include "ffmpeg_dec.h"
#include "ffmpeg_dec_audio.h"
#include "ff_rate_ctrl.h"

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct ff_dec_single_t_
{
    // 视频解码
    struct
    {
        klb_list_t*     p_video_list;   // 待解码的视频帧列表
        klb_list_t*     p_yuv_list;     // 解码完毕的视频yuv数据列表

        ffmpeg_dec_t*   p_dec;          // 解码器
        enum AVCodecID  dec_code_id;    // 解码器ID

        bool            need_key;       // 需要关键帧
    };

    // 音频解码
    struct
    {
        klb_list_t*         p_audio_list;   // 待解码的音频帧列表
        klb_list_t*         p_wav_list;     // 解码完毕的音频WAV数据列表

        ffmpeg_dec_audio_t* p_dec_audio;    // 音频解码器
    };

    // 速率/丢帧 控制
    struct
    {
        ff_rate_ctrl_t* p_rate_ctrl_active;
        ff_rate_ctrl_t* p_rate_ctrl[FF_RATE_CTRL_MAX];
    };

#ifdef __EMSCRIPTEN_PTHREADS__
    klb_thread_t*   p_thread;
#endif
}ff_dec_single_t;


ff_dec_single_t* ff_dec_single_create();
void ff_dec_single_destroy(ff_dec_single_t* p_ff_dec);

int ff_dec_single_start(ff_dec_single_t* p_ff_dec);
void ff_dec_single_stop(ff_dec_single_t* p_ff_dec);
int ff_dec_single_run(ff_dec_single_t* p_ff_dec, int64_t now_ticks, bool hidden);


/// @brief 放入数据: 视频/音频
/// @param [in] *p_manage       模块
/// @param [in] *p_buf          音视频数据
/// @return 0.成功; 非0.失败
int ff_dec_single_push(ff_dec_single_t* p_ff_dec, em_buf_t* p_buf);


em_frame_yuv_wav_t* ff_dec_single_get(ff_dec_single_t* p_ff_dec, int64_t now_ticks);
void ff_dec_single_free(em_frame_yuv_wav_t* p_yuv);


/// @brief 设置播放延时
void ff_dec_single_set_delay(ff_dec_single_t* p_ff_dec, ff_rate_ctrl_type_e delay);


#ifdef __cplusplus
}
#endif

#endif // __FF_DEC_SINGLE_H__
//end
