#ifndef __EM_FETCH_STREAM_H__
#define __EM_FETCH_STREAM_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif

// -s FETCH=1
// 标准fetch库, 不是适合做流处理, 所以加入专门针对流处理的附加库 libmnpfetch.js

typedef struct emfetch_stream_t_ emfetch_stream_t;

typedef int(*emfetch_stream_open_cb)(emfetch_stream_t* p_emfetch, uint32_t now_ticks, int code);
typedef int(*emfetch_stream_error_cb)(emfetch_stream_t* p_emfetch, uint32_t now_ticks);
typedef int(*emfetch_stream_recv_cb)(emfetch_stream_t* p_emfetch, uint32_t now_ticks, const char* p_data, int data_len);


typedef struct emfetch_stream_t_
{
    int                         id;

    void*                       p_user1;    ///< 用户自定义指针1
    void*                       p_user2;    ///< 用户自定义指针2

    emfetch_stream_open_cb      cb_open;
    emfetch_stream_error_cb     cb_error;
    emfetch_stream_recv_cb      cb_recv;    ///< 搭配 em_socket_manage_t 时使用; 当网络上有数据可以接收时, 被调用

    int                         status;     ///< 状态
#define EMFETCH_STREAM_STATUS_NULL      0
#define EMFETCH_STREAM_STATUS_OPEN      1
#define EMFETCH_STREAM_STATUS_ERROR     2
}emfetch_stream_t;


emfetch_stream_t* emfetch_stream_create();
void emfetch_stream_destroy(emfetch_stream_t* p_emfetch);

int emfetch_stream_connect(emfetch_stream_t* p_emfetch, const char* p_method, const char* p_url);



#ifdef __cplusplus
}
#endif

#endif // __EM_FETCH_STREAM_H__
//end
