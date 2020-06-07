///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    em_conn_manage.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __EM_CONN_MANAGE_H__
#define __EM_CONN_MANAGE_H__

#include "klb_type.h"
#include "em_net/em_socket.h"
#include "em_ws/em_ws.h"
#include "em_fetch/em_fetch_stream.h"
#include "em_net/em_socket_manage.h"
#include "em_util/em_buf.h"

#if defined(__cplusplus)
extern "C" {
#endif


#define EM_NET_NAME_LEN     16                      ///< 名称长度
#define EM_NET_NAME_BUF     (EM_NET_NAME_LEN + 4)   ///< 名称缓存长度


#define EM_NET_HTTP_FLV     "HTTP-FLV"              ///< HTTP FLV
#define EM_NET_HTTP_MNP     "HTTP-MNP"              ///< HTTP MNP

#define EM_NET_WS_FLV       "WS-FLV"                ///< Websocket FLV
#define EM_NET_WS_MNP       "WS-MNP"                ///< Websocket MNP


typedef enum em_conn_manage_code_e_
{
    EM_CMC_OK               = 0,                    ///< 成功
    EM_CMC_NEW_CONNECT      = 1,                    ///< 新连接成功

    EM_CMC_ERR_NEW_CONNECT  = 11,                   ///< 新连接失败
    EM_CMC_ERR_DISCONNECT   = 12,                   ///< 连接断开
    EM_CMC_ERR_TIME_OUT     = 13,                   ///< 超时断开
}em_conn_manage_code_e;


typedef void(*em_conn_env_destroy_cb)(void* ptr);
typedef int(*em_conn_env_send_cb)(void* ptr, em_buf_t* p_buf);
typedef int(*em_conn_env_send_md_cb)(void* ptr);

typedef struct em_conn_env_t_
{
    void*                   p_conn;
    em_socket_t*            p_socket;
    emws_socket_t*          p_emws;
    emfetch_stream_t*       p_emfetch;
    const char*             p_protocol;

    em_conn_env_destroy_cb  cb_destroy;
    em_conn_env_send_cb     cb_send;
    em_conn_env_send_md_cb  cb_send_md;
}em_conn_env_t;


typedef struct em_conn_manage_t_ em_conn_manage_t;


em_conn_manage_t* em_conn_manage_create(em_socket_manage_t* p_socket_manage);
void em_conn_manage_destroy(em_conn_manage_t* p_manage);

int em_conn_manage_connect(em_conn_manage_t* p_manage, const char* p_protocol, const char* p_name, const char* p_ip, int port, const char* p_path);
int em_conn_manage_close(em_conn_manage_t* p_manage, const char* p_name);

int em_conn_manage_send(em_conn_manage_t* p_manage, const char* p_name, em_buf_t* p_buf);
int em_conn_manage_send_txt(em_conn_manage_t* p_manage, const char* p_name, const char* p_data);

int em_conn_manage_recv(em_conn_manage_t* p_manage, char* p_protocol, char* p_name, em_buf_t** p_buf, int* p_code);
int em_conn_manage_recv_md(em_conn_manage_t* p_manage, char* p_protocol, char* p_name, em_buf_t** p_buf);
//int em_conn_manage_send_md(em_conn_manage_t* p_manage);


#ifdef __cplusplus
}
#endif

#endif // __EM_CONN_MANAGE_H__
//end
