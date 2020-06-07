///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    main_app_in.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __MAIN_APP_IN_H__
#define __MAIN_APP_IN_H__

#include "klb_type.h"
#include "test_res/tmem_h264.h"
#include "em_net/em_socket_manage.h"
#include "em_net/em_conn_manage.h"
#include "ff_dec/ff_dec_manage.h"
#include "em_gl/gl_ctx.h"
#include "em_gl/gl_yuv.h"
#include "main_app/main_app_task.h"
#include <assert.h>


#if defined(__cplusplus)
extern "C" {
#endif


typedef struct main_app_t_
{
    // 网络
    em_socket_manage_t* p_socket_manage;
    em_conn_manage_t*   p_conn_manage;

    // 显示
    emgl_ctx_t*         p_gl_ctx;
    emgl_yuv_t*         p_gl_yuv;

    // 解码
    ff_dec_manage_t*    p_dec_manage;
    //ffmpeg_dec_t*     p_ffmpeg_dec;

    // 测试h264数据
    tmem_h264_t*        p_tmem_h264;
    uint32_t            last_tc;

    main_app_task_t*    p_open_task;
    main_app_task_t*    p_request_task;
}main_app_t;



#ifdef __cplusplus
}
#endif

#endif // __MAIN_APP_IN_H__
//end
