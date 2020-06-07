#include "em_conn_ws_mnp.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"
#include "list/klb_list.h"
#include <assert.h>

typedef struct em_conn_ws_mnp_t_
{
    em_conn_manage_t*   p_conn_manage;
    em_socket_t*        p_socket;

    klb_list_t*         p_wlist_no_md;
    em_buf_t*           p_wbuf_no_md;
    int                 pos_wbuf_no_md;
}em_conn_ws_mnp_t;

void em_conn_ws_mnp_init_env(em_conn_env_t* p_env, em_conn_ws_mnp_t* p_mnp)
{
    em_socket_t* p_socket = p_mnp->p_socket;

    p_env->p_conn = p_mnp;
    p_env->p_socket = p_socket;
    p_env->p_emws = NULL;
    p_env->p_protocol = EM_NET_WS_MNP;

    p_env->cb_destroy = em_conn_ws_mnp_destroy;
    p_env->cb_send = em_conn_ws_mnp_send;
    p_env->cb_send_md = em_conn_ws_mnp_send_md;

    p_socket->p_user1 = p_env;
    p_socket->p_user2 = p_mnp;
    p_socket->cb_recv = em_conn_ws_mnp_on_recv;
    p_socket->cb_send = em_conn_ws_mnp_on_send;
}

em_conn_ws_mnp_t* em_conn_ws_mnp_create(em_conn_manage_t* p_conn_manage, em_socket_t* p_socket)
{
    em_conn_ws_mnp_t* p_mnp = KLB_MALLOC(em_conn_ws_mnp_t, 1, 0);
    KLB_MEMSET(p_mnp, 0, sizeof(em_conn_ws_mnp_t));

    p_mnp->p_conn_manage = p_conn_manage;
    p_mnp->p_socket = p_socket;

    p_mnp->p_wlist_no_md = klb_list_create();

    return p_mnp;
}

void em_conn_ws_mnp_destroy(void* ptr)
{
    em_conn_ws_mnp_t* p_mnp = (em_conn_ws_mnp_t*)ptr;
    assert(NULL != p_mnp);

    // 释放来不及发送的数据
    if (NULL != p_mnp->p_wbuf_no_md)
    {
        em_buf_unref_next(p_mnp->p_wbuf_no_md);
        p_mnp->p_wbuf_no_md = NULL;
    }

    while (0 < klb_list_size(p_mnp->p_wlist_no_md))
    {
        em_buf_t* p_tmp = (em_buf_t*)klb_list_pop_head(p_mnp->p_wlist_no_md);
        em_buf_unref_next(p_tmp);
    }

    KLB_FREE_BY(p_mnp->p_wlist_no_md, klb_list_destroy);
    KLB_FREE(p_mnp);
}

int em_conn_ws_mnp_send(void* ptr, em_buf_t* p_buf)
{
    em_conn_ws_mnp_t* p_mnp = (em_conn_ws_mnp_t*)ptr;
    assert(NULL != p_mnp);

    em_buf_ref_next(p_buf);
    klb_list_push_tail(p_mnp->p_wlist_no_md, p_buf);

    em_socket_set_flag_write(p_mnp->p_socket);

    return 0;
}

int em_conn_ws_mnp_send_md(void* ptr)
{
    em_conn_ws_mnp_t* p_mnp = (em_conn_ws_mnp_t*)ptr;
    assert(NULL != p_mnp);

    return 0;
}

int em_conn_ws_mnp_on_recv(em_socket_t* p_socket, uint32_t now_ticks)
{
    assert(NULL != p_socket);
    em_conn_env_t* p_env = (em_conn_env_t*)p_socket->p_user1;
    em_conn_ws_mnp_t* p_mnp = (em_conn_ws_mnp_t*)p_socket->p_user2;
    assert(NULL != p_mnp);

    int recv = 0;

    char buf[4096] = { 0 };
    recv = em_socket_recv(p_socket, (uint8_t*)buf, 4095);

    if (0 < recv)
    {
        LOG("ws mnp recv(%d):[%s]", recv, buf);
    }

    return recv;
}

static int em_conn_ws_mnp_send_no_md(em_conn_ws_mnp_t* p_mnp, em_socket_t* p_socket)
{
    em_buf_t* p_buf = p_mnp->p_wbuf_no_md;
    if (NULL == p_buf)
    {
        return 0; // 当前无数据发送
    }

    int send = 0;

    uint8_t* p_data = p_buf->p_buf + p_buf->start;
    int data_len = p_buf->end - p_mnp->pos_wbuf_no_md;

    if (0 < data_len)
    {
        int n = em_socket_send(p_socket, p_data, data_len);
        p_mnp->pos_wbuf_no_md += n;
        send += n;
    }

    if (p_buf->end <= p_mnp->pos_wbuf_no_md)
    {
        p_mnp->p_wbuf_no_md = p_buf->p_next;
        p_mnp->pos_wbuf_no_md = (NULL != p_mnp->p_wbuf_no_md) ? p_mnp->p_wbuf_no_md->start : 0;

        // 发送完毕, 释放
        em_buf_unref(p_buf);
    }

    return send;
}

int em_conn_ws_mnp_on_send(em_socket_t* p_socket, uint32_t now_ticks)
{
    assert(NULL != p_socket);
    em_conn_env_t* p_env = (em_conn_env_t*)p_socket->p_user1;
    em_conn_ws_mnp_t* p_mnp = (em_conn_ws_mnp_t*)p_socket->p_user2;
    assert(NULL != p_mnp);

    int send = 0;

    send += em_conn_ws_mnp_send_no_md(p_mnp, p_socket);

    if (NULL == p_mnp->p_wbuf_no_md)
    {
        p_mnp->p_wbuf_no_md = (em_buf_t*)klb_list_pop_head(p_mnp->p_wlist_no_md);
        p_mnp->pos_wbuf_no_md = (NULL != p_mnp->p_wbuf_no_md) ? p_mnp->p_wbuf_no_md->start : 0;
    }

    if (NULL == p_mnp->p_wbuf_no_md && 0 == klb_list_size(p_mnp->p_wlist_no_md))
    {
        // 无数据可发送
        em_socket_clear_flag_write(p_socket);
    }

    return send;
}
