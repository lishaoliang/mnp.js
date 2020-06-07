///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    em_socket_manage.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __EM_SOCKET_MANAGE_H__
#define __EM_SOCKET_MANAGE_H__

#include "klb_type.h"
#include "em_net/em_socket.h"
#include "em_ws/em_ws.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct em_socket_manage_t_ em_socket_manage_t;


em_socket_manage_t* em_socket_manage_create();
void em_socket_manage_destroy(em_socket_manage_t* p_manage);

int em_socket_manage_run(em_socket_manage_t* p_manage, uint32_t now_ticks);

int em_socket_manage_push(em_socket_manage_t* p_manage, const void* p_key, uint32_t key_len, em_socket_t* p_socket);
int em_socket_manage_push_ws(em_socket_manage_t* p_manage, const void* p_key, uint32_t key_len, emws_socket_t* p_emws);
int em_socket_manage_remove(em_socket_manage_t* p_manage, const void* p_key, uint32_t key_len);

#ifdef __cplusplus
}
#endif

#endif // __EM_SOCKET_MANAGE_H__
//end
