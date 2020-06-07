///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    gl_yuv.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __GL_YUV_H__
#define __GL_YUV_H__

#include "klb_type.h"
#include "em_util/em_yuv_frame.h"

typedef struct emgl_yuv_t_ emgl_yuv_t;

emgl_yuv_t* emgl_yuv_create();
void emgl_yuv_destroy(emgl_yuv_t* p_rgb);


int emgl_yuv_draw(emgl_yuv_t* p_yuv, em_yuv_frame_t* p_frame);


#endif // __GL_YUV_H__
//end
