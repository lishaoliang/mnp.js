#include "em_fetch_mnp.h"
#include "em_conn_manage_in.h"
#include "mnp/klb_mnp.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"
#include "em_mnp_demux.h"
#include "list/klb_list.h"
#include <assert.h>


typedef struct emfetch_mnp_t_
{
    char                name[EM_NET_NAME_BUF];

    em_conn_manage_t*   p_conn_manage;
    emfetch_stream_t*   p_emfetch;

    em_mnp_demux_t*     p_mnp_demux;
    klb_list_t*         p_list_frame;
}emfetch_mnp_t;


void emfetch_mnp_init_env(em_conn_env_t* p_env, emfetch_mnp_t* p_mnp)
{
    emfetch_stream_t* p_emfetch = p_mnp->p_emfetch;

    p_env->p_conn = p_mnp;
    p_env->p_socket = NULL;
    p_env->p_emws = NULL;
    p_env->p_emfetch = p_emfetch;
    p_env->p_protocol = EM_NET_HTTP_MNP;

    p_env->cb_destroy = emfetch_mnp_destroy;
    p_env->cb_send = NULL;
    p_env->cb_send_md = NULL;

    p_emfetch->p_user1 = p_env;
    p_emfetch->p_user2 = p_mnp;
    p_emfetch->cb_open = emfetch_mnp_on_open;
    p_emfetch->cb_error = emfetch_mnp_on_error;
    p_emfetch->cb_recv = emfetch_mnp_on_recv;
}

emfetch_mnp_t* emfetch_mnp_create(const char* p_name, em_conn_manage_t* p_conn_manage, emfetch_stream_t* p_emfetch)
{
    emfetch_mnp_t* p_mnp = KLB_MALLOC(emfetch_mnp_t, 1, 0);
    KLB_MEMSET(p_mnp, 0, sizeof(emfetch_mnp_t));

    strncpy(p_mnp->name, p_name, EM_NET_NAME_LEN);

    p_mnp->p_conn_manage = p_conn_manage;
    p_mnp->p_emfetch = p_emfetch;

    p_mnp->p_mnp_demux = em_mnp_demux_create();
    p_mnp->p_list_frame = klb_list_create();

    return p_mnp;
}

void emfetch_mnp_destroy(void* ptr)
{
    emfetch_mnp_t* p_mnp = (emfetch_mnp_t*)ptr;
    assert(NULL != p_mnp);

    // 先关闭 fetch
    KLB_FREE_BY(p_mnp->p_emfetch, emfetch_stream_destroy);

    //
    while (0 < klb_list_size(p_mnp->p_list_frame))
    {
        em_buf_t* p_tmp = (em_buf_t*)klb_list_pop_head(p_mnp->p_list_frame);
        KLB_FREE_BY(p_tmp, em_buf_unref_next);
    }

    KLB_FREE_BY(p_mnp->p_mnp_demux, em_mnp_demux_destroy);
    KLB_FREE_BY(p_mnp->p_list_frame, klb_list_destroy);
    KLB_FREE(p_mnp);
}

int emfetch_mnp_on_open(emfetch_stream_t* p_emfetch, uint32_t now_ticks, int code)
{
    assert(NULL != p_emfetch);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emfetch->p_user1;
    emfetch_mnp_t* p_mnp = (emfetch_mnp_t*)p_emfetch->p_user2;
    assert(NULL != p_mnp);

    if (0 == code)
    {
        em_conn_manage_push(p_mnp->p_conn_manage, EM_NET_HTTP_MNP, p_mnp->name, EM_CMC_NEW_CONNECT, NULL);
    }
    else
    {
        em_conn_manage_push(p_mnp->p_conn_manage, EM_NET_HTTP_MNP, p_mnp->name, EM_CMC_ERR_NEW_CONNECT, NULL);
    }

    return 0;
}

int emfetch_mnp_on_error(emfetch_stream_t* p_emfetch, uint32_t now_ticks)
{
    assert(NULL != p_emfetch);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emfetch->p_user1;
    emfetch_mnp_t* p_mnp = (emfetch_mnp_t*)p_emfetch->p_user2;
    assert(NULL != p_mnp);

    em_conn_manage_push(p_mnp->p_conn_manage, EM_NET_HTTP_MNP, p_mnp->name, EM_CMC_ERR_DISCONNECT, NULL);

    return 0;
}

int emfetch_mnp_on_recv(emfetch_stream_t* p_emfetch, uint32_t now_ticks, const char* p_data, int data_len)
{
    assert(NULL != p_emfetch);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emfetch->p_user1;
    emfetch_mnp_t* p_mnp = (emfetch_mnp_t*)p_emfetch->p_user2;
    assert(NULL != p_mnp);

    int ret = em_mnp_demux_do(p_mnp->p_mnp_demux, p_data, data_len, p_mnp->p_list_frame);

    while (0 < klb_list_size(p_mnp->p_list_frame))
    {
        em_buf_t* p_frame = (em_buf_t*)klb_list_pop_head(p_mnp->p_list_frame);
        em_conn_manage_push_md(p_mnp->p_conn_manage, EM_NET_HTTP_MNP, p_mnp->name, p_frame);
    }

    if (ret < 0)
    {
        // 数据错误, 断开连接
        em_conn_manage_push(p_mnp->p_conn_manage, EM_NET_HTTP_MNP, p_mnp->name, EM_CMC_ERR_DISCONNECT, NULL);
    }

    return data_len;
}
