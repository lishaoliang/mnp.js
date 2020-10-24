#include "em_ws_flv.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"
#include "list/klb_list.h"
#include "em_util/em_buf.h"
#include "mnp/klb_mnp.h"
#include "em_net/em_conn_manage_in.h"
#include "em_flv_demux.h"
#include <assert.h>

typedef struct emws_flv_t_
{
    char                name[EM_NET_NAME_BUF];

    em_conn_manage_t*   p_conn_manage;
    emws_socket_t*      p_emws;

    em_flv_demux_t*     p_flv_demux;
    klb_list_t*         p_list_frame;
}emws_flv_t;

void emws_flv_init_env(em_conn_env_t* p_env, emws_flv_t* p_wsflv)
{
    emws_socket_t* p_emws = p_wsflv->p_emws;

    p_env->p_conn = p_wsflv;
    p_env->p_socket = NULL;
    p_env->p_emws = p_emws;
    p_env->p_emfetch = NULL;
    p_env->p_protocol = EM_NET_WS_FLV;

    p_env->cb_destroy = emws_flv_destroy;
    p_env->cb_send = emws_flv_send;
    p_env->cb_send_md = emws_flv_send_md;

    p_emws->p_user1 = p_env;
    p_emws->p_user2 = p_wsflv;
    p_emws->cb_open = emws_flv_on_open;
    p_emws->cb_error = emws_flv_on_error;
    p_emws->cb_close = emws_flv_on_close;
    p_emws->cb_recv = emws_flv_on_recv;
    p_emws->cb_send = emws_flv_on_send;
}

emws_flv_t* emws_flv_create(const char* p_name, em_conn_manage_t* p_conn_manage, emws_socket_t* p_emws)
{
    emws_flv_t* p_wsflv = KLB_MALLOC(emws_flv_t, 1, 0);
    KLB_MEMSET(p_wsflv, 0, sizeof(emws_flv_t));

    strncpy(p_wsflv->name, p_name, EM_NET_NAME_LEN);

    p_wsflv->p_conn_manage = p_conn_manage;
    p_wsflv->p_emws = p_emws;

    p_wsflv->p_flv_demux = em_flv_demux_create();
    p_wsflv->p_list_frame = klb_list_create();

    return p_wsflv;
}

void emws_flv_destroy(void* ptr)
{
    emws_flv_t* p_wsflv = (emws_flv_t*)ptr;
    assert(NULL != p_wsflv);

    while (0 < klb_list_size(p_wsflv->p_list_frame))
    {
        em_buf_t* p_tmp = (em_buf_t*)klb_list_pop_head(p_wsflv->p_list_frame);
        KLB_FREE_BY(p_tmp, em_buf_unref_next);
    }

    KLB_FREE_BY(p_wsflv->p_flv_demux, em_flv_demux_destroy);
    KLB_FREE_BY(p_wsflv->p_list_frame, klb_list_destroy);
    KLB_FREE(p_wsflv);
}

int emws_flv_send(void* ptr, em_buf_t* p_buf)
{
    emws_flv_t* p_wsflv = (emws_flv_t*)ptr;
    assert(NULL != p_wsflv);

    // 客户端flv只需处理读取即可

    return 0;
}

int emws_flv_send_md(void* ptr)
{
    emws_flv_t* p_wsflv = (emws_flv_t*)ptr;
    assert(NULL != p_wsflv);

    // 客户端flv只需处理读取即可

    return 0;
}

int emws_flv_on_recv(emws_socket_t* p_emws, uint32_t now_ticks, const EmscriptenWebSocketMessageEvent* p_message)
{
    assert(NULL != p_emws);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emws->p_user1;
    emws_flv_t* p_wsflv = (emws_flv_t*)p_emws->p_user2;
    assert(NULL != p_wsflv);

    char* p_data = (char*)p_message->data;
    int data_len = p_message->numBytes;
    //LOG("emws_flv_on_recv:%d", data_len);

    if (0 < data_len && !p_message->isText)
    {
        // 只处理二进制
        int ret = em_flv_demux_do(p_wsflv->p_flv_demux, p_data, data_len, p_wsflv->p_list_frame);

        while (0 < klb_list_size(p_wsflv->p_list_frame))
        {
            em_buf_t* p_frame = (em_buf_t*)klb_list_pop_head(p_wsflv->p_list_frame);
            em_conn_manage_push_md(p_wsflv->p_conn_manage, EM_NET_WS_FLV, p_wsflv->name, p_frame);
        }

        if (ret < 0)
        {
            // 数据错误, 断开连接
            em_conn_manage_push(p_wsflv->p_conn_manage, EM_NET_WS_FLV, p_wsflv->name, EM_CMC_ERR_DISCONNECT, NULL);
        }
    }

    return data_len;
}

int emws_flv_on_open(emws_socket_t* p_emws, uint32_t now_ticks, int code)
{
    assert(NULL != p_emws);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emws->p_user1;
    emws_flv_t* p_wsflv = (emws_flv_t*)p_emws->p_user2;
    assert(NULL != p_wsflv);

    if (0 == code)
    {
        em_conn_manage_push(p_wsflv->p_conn_manage, EM_NET_WS_FLV, p_wsflv->name, EM_CMC_NEW_CONNECT, NULL);
    }
    else
    {
        em_conn_manage_push(p_wsflv->p_conn_manage, EM_NET_WS_FLV, p_wsflv->name, EM_CMC_ERR_NEW_CONNECT, NULL);
    }

    return 0;
}

int emws_flv_on_error(emws_socket_t* p_emws, uint32_t now_ticks)
{
    assert(NULL != p_emws);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emws->p_user1;
    emws_flv_t* p_wsflv = (emws_flv_t*)p_emws->p_user2;
    assert(NULL != p_wsflv);

    em_conn_manage_push(p_wsflv->p_conn_manage, EM_NET_WS_FLV, p_wsflv->name, EM_CMC_ERR_DISCONNECT, NULL);

    return 0;
}

int emws_flv_on_close(emws_socket_t* p_emws, uint32_t now_ticks)
{
    assert(NULL != p_emws);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emws->p_user1;
    emws_flv_t* p_wsflv = (emws_flv_t*)p_emws->p_user2;
    assert(NULL != p_wsflv);

    em_conn_manage_push(p_wsflv->p_conn_manage, EM_NET_WS_FLV, p_wsflv->name, EM_CMC_ERR_DISCONNECT, NULL);

    return 0;
}

int emws_flv_on_send(emws_socket_t* p_emws, uint32_t now_ticks)
{
    assert(NULL != p_emws);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emws->p_user1;
    emws_flv_t* p_wsflv = (emws_flv_t*)p_emws->p_user2;
    assert(NULL != p_wsflv);

    return 0;
}
