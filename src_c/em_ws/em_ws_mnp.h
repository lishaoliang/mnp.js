///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    em_ws_mnp.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __EM_WS_MNP_H__
#define __EM_WS_MNP_H__

#include "klb_type.h"
#include "em_ws/em_ws.h"
#include "em_net/em_conn_manage.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct emws_mnp_t_ emws_mnp_t;


void emws_mnp_init_env(em_conn_env_t* p_env, emws_mnp_t* p_wsmnp);


emws_mnp_t* emws_mnp_create(const char* p_name, em_conn_manage_t* p_conn_manage, emws_socket_t* p_socket);
void emws_mnp_destroy(void* ptr);

int emws_mnp_send(void* ptr, em_buf_t* p_buf);
int emws_mnp_send_md(void* ptr);

int emws_mnp_on_open(emws_socket_t* p_emws, uint32_t now_ticks, int code);
int emws_mnp_on_error(emws_socket_t* p_emws, uint32_t now_ticks);
int emws_mnp_on_close(emws_socket_t* p_emws, uint32_t now_ticks);
int emws_mnp_on_recv(emws_socket_t* p_emws, uint32_t now_ticks, const EmscriptenWebSocketMessageEvent* p_message);
int emws_mnp_on_send(emws_socket_t* p_socket, uint32_t now_ticks);


#ifdef __cplusplus
}
#endif

#endif // __EM_WS_MNP_H__
//end
