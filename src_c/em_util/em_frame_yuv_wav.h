///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    em_frame_yuv_wav.h
/// @brief   YUV图像/WAV音频基础数据定义
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __EM_FRAME_YUV_WAV_H__
#define __EM_FRAME_YUV_WAV_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct em_frame_yuv_wav_t_
{
    int         type;                   ///< 类型
#define EM_FRAME_TYPE_YUV        0      ///< YUV图像
#define EM_FRAME_TYPE_WAV        1      ///< WAV音频

    int64_t     pts;                    ///< 时间戳

    union
    {
        // type = EM_FRAME_TYPE_YUV
        struct
        {
            uint8_t*    p_y;
            int         pitch_y;
            int         h_y;

            uint8_t*    p_u;
            int         pitch_u;
            int         h_u;

            uint8_t*    p_v;
            int         pitch_v;
            int         h_v;

            int         w;
            int         h;
        };

        // type = EM_FRAME_TYPE_WAV
        struct
        {
            uint8_t  sample_format;     ///< AVSampleFormat; AV_SAMPLE_FMT_S16
            uint8_t  tracks;            ///< 音频声道数; 1, 2, 5.1;
            uint32_t samples;           ///< 音频采样率; 44100
        };
    };

    uint8_t*    p_buf;                  ///< 缓存
    int         buf_len;                ///< 缓存长度

    int         start;                  ///< 有效数据起始位置
    int         end;                    ///< 有效数据结束位置
}em_frame_yuv_wav_t;


em_frame_yuv_wav_t* em_frame_yuv_wav_malloc(int type, int buf_len);

void em_frame_yuv_wav_free(em_frame_yuv_wav_t* p_frame);

#ifdef __cplusplus
}
#endif

#endif // __EM_FRAME_YUV_WAV_H__
//end
