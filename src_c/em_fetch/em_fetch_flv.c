#include "em_fetch/em_fetch_flv.h"
#include "em_net/em_conn_manage_in.h"
#include "mnp/klb_mnp.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"
#include "em_flv_demux.h"
#include "list/klb_list.h"
#include <assert.h>


typedef struct emfetch_flv_t_
{
    char                name[EM_NET_NAME_BUF];

    em_conn_manage_t*   p_conn_manage;
    emfetch_stream_t*   p_emfetch;

    em_flv_demux_t*     p_flv_demux;
    klb_list_t*         p_list_frame;
}emfetch_flv_t;


void emfetch_flv_init_env(em_conn_env_t* p_env, emfetch_flv_t* p_flv)
{
    emfetch_stream_t* p_emfetch = p_flv->p_emfetch;

    p_env->p_conn = p_flv;
    p_env->p_socket = NULL;
    p_env->p_emws = NULL;
    p_env->p_emfetch = p_emfetch;
    p_env->p_protocol = EM_NET_HTTP_FLV;

    p_env->cb_destroy = emfetch_flv_destroy;
    p_env->cb_send = NULL;
    p_env->cb_send_md = NULL;

    p_emfetch->p_user1 = p_env;
    p_emfetch->p_user2 = p_flv;
    p_emfetch->cb_open = emfetch_flv_on_open;
    p_emfetch->cb_error = emfetch_flv_on_error;
    p_emfetch->cb_recv = emfetch_flv_on_recv;
}

emfetch_flv_t* emfetch_flv_create(const char* p_name, em_conn_manage_t* p_conn_manage, emfetch_stream_t* p_emfetch)
{
    emfetch_flv_t* p_flv = KLB_MALLOC(emfetch_flv_t, 1, 0);
    KLB_MEMSET(p_flv, 0, sizeof(emfetch_flv_t));

    strncpy(p_flv->name, p_name, EM_NET_NAME_LEN);

    p_flv->p_conn_manage = p_conn_manage;
    p_flv->p_emfetch = p_emfetch;

    p_flv->p_flv_demux = em_flv_demux_create();
    p_flv->p_list_frame = klb_list_create();

    return p_flv;
}

void emfetch_flv_destroy(void* ptr)
{
    emfetch_flv_t* p_flv = (emfetch_flv_t*)ptr;
    assert(NULL != p_flv);

    // 先关闭 fetch
    KLB_FREE_BY(p_flv->p_emfetch, emfetch_stream_destroy);

    //
    while (0 < klb_list_size(p_flv->p_list_frame))
    {
        em_buf_t* p_tmp = (em_buf_t*)klb_list_pop_head(p_flv->p_list_frame);
        KLB_FREE_BY(p_tmp, em_buf_unref_next);
    }

    KLB_FREE_BY(p_flv->p_flv_demux, em_flv_demux_destroy);
    KLB_FREE_BY(p_flv->p_list_frame, klb_list_destroy);
    KLB_FREE(p_flv);
}

int emfetch_flv_on_open(emfetch_stream_t* p_emfetch, uint32_t now_ticks, int code)
{
    assert(NULL != p_emfetch);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emfetch->p_user1;
    emfetch_flv_t* p_flv = (emfetch_flv_t*)p_emfetch->p_user2;
    assert(NULL != p_flv);

    if (0 == code)
    {
        em_conn_manage_push(p_flv->p_conn_manage, EM_NET_HTTP_FLV, p_flv->name, EM_CMC_NEW_CONNECT, NULL);
    }
    else
    {
        em_conn_manage_push(p_flv->p_conn_manage, EM_NET_HTTP_FLV, p_flv->name, EM_CMC_ERR_NEW_CONNECT, NULL);
    }

    return 0;
}

int emfetch_flv_on_error(emfetch_stream_t* p_emfetch, uint32_t now_ticks)
{
    assert(NULL != p_emfetch);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emfetch->p_user1;
    emfetch_flv_t* p_flv = (emfetch_flv_t*)p_emfetch->p_user2;
    assert(NULL != p_flv);

    em_conn_manage_push(p_flv->p_conn_manage, EM_NET_HTTP_FLV, p_flv->name, EM_CMC_ERR_DISCONNECT, NULL);

    return 0;
}

int emfetch_flv_on_recv(emfetch_stream_t* p_emfetch, uint32_t now_ticks, const char* p_data, int data_len)
{
    assert(NULL != p_emfetch);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emfetch->p_user1;
    emfetch_flv_t* p_flv = (emfetch_flv_t*)p_emfetch->p_user2;
    assert(NULL != p_flv);

    int ret = em_flv_demux_do(p_flv->p_flv_demux, p_data, data_len, p_flv->p_list_frame);

    while (0 < klb_list_size(p_flv->p_list_frame))
    {
        em_buf_t* p_frame = (em_buf_t*)klb_list_pop_head(p_flv->p_list_frame);
        em_conn_manage_push_md(p_flv->p_conn_manage, EM_NET_HTTP_FLV, p_flv->name, p_frame);
    }

    if (ret < 0)
    {
        // 数据错误, 断开连接
        em_conn_manage_push(p_flv->p_conn_manage, EM_NET_HTTP_FLV, p_flv->name, EM_CMC_ERR_DISCONNECT, NULL);
    }

    return data_len;
}
