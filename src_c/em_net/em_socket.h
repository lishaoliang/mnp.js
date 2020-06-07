///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    em_socket.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __EM_SOCKET_H__
#define __EM_SOCKET_H__

#include "klb_type.h"
#include <sys/socket.h>
#include <netinet/in.h>

#if defined(__cplusplus)
extern "C" {
#endif


#define EM_SOCKET_HASH_TABLE_MAX    20000


typedef int                 em_socket_fd;
#define INVALID_SOCKET      -1
typedef struct em_socket_t_ em_socket_t;


typedef enum em_socket_flag_e_
{
    EM_SOCKET_WRITE = 0x0001        ///< 有数据写标记
}em_socket_flag_e;


typedef int(*em_socket_recv_cb)(em_socket_t* p_socket, uint32_t now_ticks);
typedef int(*em_socket_send_cb)(em_socket_t* p_socket, uint32_t now_ticks);


typedef struct em_socket_t_
{
    em_socket_fd        fd;
    uint32_t            flag;
    uint32_t            tickcount;

    void*               p_user1;    ///< 用户自定义指针1
    void*               p_user2;    ///< 用户自定义指针2

    em_socket_recv_cb   cb_recv;    ///< 搭配 em_socket_manage_t 时使用; 当网络上有数据可以接收时, 被调用
    em_socket_send_cb   cb_send;    ///< 搭配 em_socket_manage_t 时使用; 当网络上可以发送数据时, 被调用
}em_socket_t;


/// @brief 创建socket对象
/// @return em_socket_t* 对象指针
/// @note
///   eg. em_socket_t* p_socket = em_socket_create(AF_INET, SOCK_STREAM, IPPROTO_TCP)
///   https://blog.squareys.de/emscripten-sockets/#binary-subprotocol
///   https://github.com/emscripten-core/emscripten/tree/master/tests/sockets
em_socket_t* em_socket_create(int af, int type, int protocol);
void em_socket_destroy(em_socket_t* p_socket);


/// @brief 向某ip:port发起连接
/// @return int 0.成功; 非0.失败
/// @note
///   浏览器会直接发起websocket连接
int em_socket_connect(em_socket_t* p_socket, const char* p_ip, int port);


/// @brief 发送数据
/// @param [in] *p_socket   socket对象
/// @param [in] *p_buf      缓存数据
/// @param [in] len         数据长度
/// @return int 发送的数据长度
/// @note 浏览器内部直接按 Websoket[Binary] + [Data] 的方式发送一个片段
///   这里只能认为是基于Websoket-Binary的透明传输, 只保证了字节次序
///   在数据接收方依然需要做更高层的协议判定
int em_socket_send(em_socket_t* p_socket, const uint8_t* p_buf, int len);
int em_socket_recv(em_socket_t* p_socket, uint8_t* p_buf, int len);

int em_socket_sendto(em_socket_t* p_socket, const uint8_t* p_buf, int len, const struct sockaddr* p_addr, int addr_len);
int em_socket_recvfrom(em_socket_t* p_socket, uint8_t* p_buf, int len, struct sockaddr* p_addr, int* p_addr_len);

void em_socket_set_flag_write(em_socket_t* p_socket);
void em_socket_clear_flag_write(em_socket_t* p_socket);

#ifdef __cplusplus
}
#endif


#endif // __EM_SOCKET_H__
//end
