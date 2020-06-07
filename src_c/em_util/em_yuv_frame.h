///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    em_yuv_frame.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __EM_YUV_FRAME_H__
#define __EM_YUV_FRAME_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct em_yuv_frame_t_
{
    int64_t     pts;        // 时间戳

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

    uint8_t*    p_buf;
    int         buf_len;
}em_yuv_frame_t;


#ifdef __cplusplus
}
#endif

#endif // __EM_YUV_FRAME_H__
//end
