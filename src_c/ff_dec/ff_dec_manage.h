///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    ff_dec_manage.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __FF_DEC_MANAGE_H__
#define __FF_DEC_MANAGE_H__

#include "klb_type.h"
#include "em_util/em_yuv_frame.h"
#include "em_util/em_buf.h"


#if defined(__cplusplus)
extern "C" {
#endif


typedef struct ff_dec_manage_t_ ff_dec_manage_t;


ff_dec_manage_t* ff_dec_manage_create();
void ff_dec_manage_destroy(ff_dec_manage_t* p_manage);

int ff_dec_manage_start(ff_dec_manage_t* p_manage);
void ff_dec_manage_stop(ff_dec_manage_t* p_manage);
int ff_dec_manage_run(ff_dec_manage_t* p_manage, uint32_t now_ticks);

int ff_dec_manage_push(ff_dec_manage_t* p_manage, em_buf_t* p_buf);
em_yuv_frame_t* ff_dec_manage_get(ff_dec_manage_t* p_manage);
void ff_dec_manage_free(em_yuv_frame_t* p_yuv);


#ifdef __cplusplus
}
#endif

#endif // __FF_DEC_MANAGE_H__
//end
