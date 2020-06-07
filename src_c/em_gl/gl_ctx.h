///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    gl_ctx.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __GL_CTX_H__
#define __GL_CTX_H__

#include "klb_type.h"

typedef struct emgl_ctx_t_ emgl_ctx_t;


int emgl_ctx_init(int w, int h, int bpp);


emgl_ctx_t* emgl_ctx_create();
void emgl_ctx_destroy(emgl_ctx_t* p_ctx);

void emgl_ctx_viewport(emgl_ctx_t* p_ctx, int x, int y, int w, int h);

void emgl_ctx_begin(emgl_ctx_t* p_ctx);
void emgl_ctx_end(emgl_ctx_t* p_ctx);


#endif // __GL_CTX_H__
//end
