///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    em_ws.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __EM_WS_H__
#define __EM_WS_H__

#include "klb_type.h"
#include <emscripten/websocket.h>

#if defined(__cplusplus)
extern "C" {
#endif

//if (!emscripten_websocket_is_supported())
//{
//  printf("WebSockets are not supported, cannot continue!\n");
//  exit(1);
//}

typedef struct emws_socket_t_ emws_socket_t;

typedef int(*emws_socket_open_cb)(emws_socket_t* p_emws, uint32_t now_ticks, int code);
typedef int(*emws_socket_error_cb)(emws_socket_t* p_emws, uint32_t now_ticks);
typedef int(*emws_socket_close_cb)(emws_socket_t* p_emws, uint32_t now_ticks);
typedef int(*emws_socket_recv_cb)(emws_socket_t* p_emws, uint32_t now_ticks, const EmscriptenWebSocketMessageEvent* p_message);
typedef int(*emws_socket_send_cb)(emws_socket_t* p_emws, uint32_t now_ticks);

typedef struct emws_socket_t_
{
    EMSCRIPTEN_WEBSOCKET_T      fd;         ///< fd
    uint32_t                    flag;       ///< 标记, 参考em_socket_t.flag
    uint32_t                    tickcount;

    void*                       p_user1;    ///< 用户自定义指针1
    void*                       p_user2;    ///< 用户自定义指针2

    emws_socket_open_cb         cb_open;
    emws_socket_error_cb        cb_error;
    emws_socket_close_cb        cb_close;
    emws_socket_recv_cb         cb_recv;    ///< 搭配 em_socket_manage_t 时使用; 当网络上有数据可以接收时, 被调用
    emws_socket_send_cb         cb_send;    ///< 搭配 em_socket_manage_t 时使用; 当网络上可以发送数据时, 被调用

    int                         status;     ///< 状态
}emws_socket_t;


typedef enum emws_socket_status_e_
{
    EMWS_SOCKET_NULL = 0,
    EMWS_SOCKET_OPEN,
    EMWS_SOCKET_ERROR,
    EMWS_SOCKET_CLOSE
}emws_socket_status_e;


emws_socket_t* emws_socket_create();
void emws_socket_destroy(emws_socket_t* p_emws);

int emws_socket_connect(emws_socket_t* p_emws, const char* p_ip, int port, const char* p_path);

int emws_socket_send_utf8_text(emws_socket_t* p_emws, const uint8_t* p_buf, int len);
int emws_socket_send_binary(emws_socket_t* p_emws, const uint8_t* p_buf, int len);

void emws_socket_set_flag_write(emws_socket_t* p_emws);
void emws_socket_clear_flag_write(emws_socket_t* p_emws);


#ifdef __cplusplus
}
#endif

#endif // __EM_WS_H__
//end
