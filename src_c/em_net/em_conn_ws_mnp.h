///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    em_conn_ws_mnp.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __EM_CONN_WS_MNP_H__
#define __EM_CONN_WS_MNP_H__

#include "klb_type.h"
#include "em_socket.h"
#include "em_conn_manage.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct em_conn_ws_mnp_t_ em_conn_ws_mnp_t;


void em_conn_ws_mnp_init_env(em_conn_env_t* p_env, em_conn_ws_mnp_t* p_mnp);


em_conn_ws_mnp_t* em_conn_ws_mnp_create(em_conn_manage_t* p_conn_manage, em_socket_t* p_socket);
void em_conn_ws_mnp_destroy(void* ptr);

int em_conn_ws_mnp_send(void* ptr, em_buf_t* p_buf);
int em_conn_ws_mnp_send_md(void* ptr);

int em_conn_ws_mnp_on_recv(em_socket_t* p_socket, uint32_t now_ticks);
int em_conn_ws_mnp_on_send(em_socket_t* p_socket, uint32_t now_ticks);


#ifdef __cplusplus
}
#endif

#endif // __EM_CONN_WS_MNP_H__
//end
