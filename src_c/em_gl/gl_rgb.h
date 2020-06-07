///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    gl_rgb.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __GL_RGB_H__
#define __GL_RGB_H__

#include "klb_type.h"
#include "gl_ctx.h"

typedef struct emgl_rgb_t_ emgl_rgb_t;

emgl_rgb_t* emgl_rgb_create(int w, int h);
void emgl_rgb_estroy(emgl_rgb_t* p_rgb);



int emgl_rgb_draw(emgl_rgb_t* p_rgb);



#endif // __GL_RGB_H__
//end

