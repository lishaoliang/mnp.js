#include "em_ws_mnp.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"
#include "list/klb_list.h"
#include "em_util/em_buf.h"
#include "mnp/klb_mnp.h"
#include "em_net/em_conn_manage_in.h"
#include "em_mnp_demux.h"
#include <assert.h>

typedef struct emws_mnp_t_
{
    char                name[EM_NET_NAME_BUF];

    em_conn_manage_t*   p_conn_manage;
    emws_socket_t*      p_emws;

    klb_list_t*         p_wlist_no_md;
    em_buf_t*           p_wbuf_no_md;
    int                 pos_wbuf_no_md;

    em_mnp_demux_t*     p_mnp_demux;
    klb_list_t*         p_list_frame;
}emws_mnp_t;

void emws_mnp_init_env(em_conn_env_t* p_env, emws_mnp_t* p_wsmnp)
{
    emws_socket_t* p_emws = p_wsmnp->p_emws;

    p_env->p_conn = p_wsmnp;
    p_env->p_socket = NULL;
    p_env->p_emws = p_emws;
    p_env->p_emfetch = NULL;
    p_env->p_protocol = EM_NET_WS_MNP;

    p_env->cb_destroy = emws_mnp_destroy;
    p_env->cb_send = emws_mnp_send;
    p_env->cb_send_md = emws_mnp_send_md;

    p_emws->p_user1 = p_env;
    p_emws->p_user2 = p_wsmnp;
    p_emws->cb_open = emws_mnp_on_open;
    p_emws->cb_error = emws_mnp_on_error;
    p_emws->cb_close = emws_mnp_on_close;
    p_emws->cb_recv = emws_mnp_on_recv;
    p_emws->cb_send = emws_mnp_on_send;
}

emws_mnp_t* emws_mnp_create(const char* p_name, em_conn_manage_t* p_conn_manage, emws_socket_t* p_emws)
{
    emws_mnp_t* p_wsmnp = KLB_MALLOC(emws_mnp_t, 1, 0);
    KLB_MEMSET(p_wsmnp, 0, sizeof(emws_mnp_t));

    strncpy(p_wsmnp->name, p_name, EM_NET_NAME_LEN);

    p_wsmnp->p_conn_manage = p_conn_manage;
    p_wsmnp->p_emws = p_emws;

    p_wsmnp->p_wlist_no_md = klb_list_create();

    p_wsmnp->p_mnp_demux = em_mnp_demux_create();
    p_wsmnp->p_list_frame = klb_list_create();

    return p_wsmnp;
}

void emws_mnp_destroy(void* ptr)
{
    emws_mnp_t* p_wsmnp = (emws_mnp_t*)ptr;
    assert(NULL != p_wsmnp);

    // 释放来不及发送的数据
    if (NULL != p_wsmnp->p_wbuf_no_md)
    {
        em_buf_unref_next(p_wsmnp->p_wbuf_no_md);
        p_wsmnp->p_wbuf_no_md = NULL;
    }

    while (0 < klb_list_size(p_wsmnp->p_wlist_no_md))
    {
        em_buf_t* p_tmp = (em_buf_t*)klb_list_pop_head(p_wsmnp->p_wlist_no_md);
        KLB_FREE_BY(p_tmp, em_buf_unref_next);
    }

    while (0 < klb_list_size(p_wsmnp->p_list_frame))
    {
        em_buf_t* p_tmp = (em_buf_t*)klb_list_pop_head(p_wsmnp->p_list_frame);
        KLB_FREE_BY(p_tmp, em_buf_unref_next);
    }

    KLB_FREE_BY(p_wsmnp->p_mnp_demux, em_mnp_demux_destroy);
    KLB_FREE_BY(p_wsmnp->p_list_frame, klb_list_destroy);
    KLB_FREE_BY(p_wsmnp->p_wlist_no_md, klb_list_destroy);
    KLB_FREE(p_wsmnp);
}

int emws_mnp_send(void* ptr, em_buf_t* p_buf)
{
    emws_mnp_t* p_wsmnp = (emws_mnp_t*)ptr;
    assert(NULL != p_wsmnp);

    //LOG("emws_mnp_send:%s", p_buf->p_buf);

    em_buf_ref_next(p_buf);
    klb_list_push_tail(p_wsmnp->p_wlist_no_md, p_buf);

    emws_socket_set_flag_write(p_wsmnp->p_emws);

    return 0;
}

int emws_mnp_send_md(void* ptr)
{
    emws_mnp_t* p_mnp = (emws_mnp_t*)ptr;
    assert(NULL != p_mnp);

    return 0;
}

int emws_mnp_on_recv(emws_socket_t* p_emws, uint32_t now_ticks, const EmscriptenWebSocketMessageEvent* p_message)
{
    assert(NULL != p_emws);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emws->p_user1;
    emws_mnp_t* p_wsmnp = (emws_mnp_t*)p_emws->p_user2;
    assert(NULL != p_wsmnp);

    char* p_data = (char*)p_message->data;
    int data_len = p_message->numBytes;

    if (0 < data_len && !p_message->isText)
    {
        // 只处理二进制, 仅认为: websocket做透明传输, 仅保证了字节流的次序
        // 这里需要再次分析协议数据, 不能依赖websocket的包具有某种结构
        // 原则上此方法可以处理具有分包信息的任何上层协议
        // 例如: WS-FLV, 甚至RTSP/RTMP等等
        // 服务端也仅需遵守websocket部分规范即可

        int ret = em_mnp_demux_do(p_wsmnp->p_mnp_demux, p_data, data_len, p_wsmnp->p_list_frame);

        while (0 < klb_list_size(p_wsmnp->p_list_frame))
        {
            em_buf_t* p_frame = (em_buf_t*)klb_list_pop_head(p_wsmnp->p_list_frame);
            em_conn_manage_push_md(p_wsmnp->p_conn_manage, EM_NET_WS_MNP, p_wsmnp->name, p_frame);
        }

        if (ret < 0)
        {
            // 数据错误, 断开连接
            em_conn_manage_push(p_wsmnp->p_conn_manage, EM_NET_WS_MNP, p_wsmnp->name, EM_CMC_ERR_DISCONNECT, NULL);
        }
    }

    return data_len;
}

static int emws_mnp_send_no_md(emws_mnp_t* p_wsmnp, emws_socket_t* p_emws)
{
    em_buf_t* p_buf = p_wsmnp->p_wbuf_no_md;
    if (NULL == p_buf)
    {
        return 0; // 当前无数据发送
    }

    int send = 0;

    uint8_t* p_data = p_buf->p_buf + p_buf->start;
    int data_len = p_buf->end - p_wsmnp->pos_wbuf_no_md;

    if (0 < data_len)
    {
        // 只使用 ws-binary 收发数据
        int n = emws_socket_send_binary(p_emws, p_data, data_len);
        //LOG("emws_mnp_send_no_md send binary:[%d],[%d]", n, data_len);

        p_wsmnp->pos_wbuf_no_md += n;
        send += n;
    }

    if (p_buf->end <= p_wsmnp->pos_wbuf_no_md)
    {
        p_wsmnp->p_wbuf_no_md = p_buf->p_next;
        p_wsmnp->pos_wbuf_no_md = (NULL != p_wsmnp->p_wbuf_no_md) ? p_wsmnp->p_wbuf_no_md->start : 0;

        // 发送完毕, 释放
        em_buf_unref(p_buf);
    }

    return send;
}

int emws_mnp_on_open(emws_socket_t* p_emws, uint32_t now_ticks, int code)
{
    assert(NULL != p_emws);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emws->p_user1;
    emws_mnp_t* p_wsmnp = (emws_mnp_t*)p_emws->p_user2;
    assert(NULL != p_wsmnp);

    if (0 == code)
    {
        em_conn_manage_push(p_wsmnp->p_conn_manage, EM_NET_WS_MNP, p_wsmnp->name, EM_CMC_NEW_CONNECT, NULL);
    }
    else
    {
        em_conn_manage_push(p_wsmnp->p_conn_manage, EM_NET_WS_MNP, p_wsmnp->name, EM_CMC_ERR_NEW_CONNECT, NULL);
    }

    return 0;
}

int emws_mnp_on_error(emws_socket_t* p_emws, uint32_t now_ticks)
{
    assert(NULL != p_emws);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emws->p_user1;
    emws_mnp_t* p_wsmnp = (emws_mnp_t*)p_emws->p_user2;
    assert(NULL != p_wsmnp);

    em_conn_manage_push(p_wsmnp->p_conn_manage, EM_NET_WS_MNP, p_wsmnp->name, EM_CMC_ERR_DISCONNECT, NULL);

    return 0;
}

int emws_mnp_on_close(emws_socket_t* p_emws, uint32_t now_ticks)
{
    assert(NULL != p_emws);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emws->p_user1;
    emws_mnp_t* p_wsmnp = (emws_mnp_t*)p_emws->p_user2;
    assert(NULL != p_wsmnp);

    em_conn_manage_push(p_wsmnp->p_conn_manage, EM_NET_WS_MNP, p_wsmnp->name, EM_CMC_ERR_DISCONNECT, NULL);

    return 0;
}

int emws_mnp_on_send(emws_socket_t* p_emws, uint32_t now_ticks)
{
    assert(NULL != p_emws);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emws->p_user1;
    emws_mnp_t* p_wsmnp = (emws_mnp_t*)p_emws->p_user2;
    assert(NULL != p_wsmnp);

    int send = 0;

    send += emws_mnp_send_no_md(p_wsmnp, p_emws);

    if (NULL == p_wsmnp->p_wbuf_no_md)
    {
        p_wsmnp->p_wbuf_no_md = (em_buf_t*)klb_list_pop_head(p_wsmnp->p_wlist_no_md);
        p_wsmnp->pos_wbuf_no_md = (NULL != p_wsmnp->p_wbuf_no_md) ? p_wsmnp->p_wbuf_no_md->start : 0;
    }

    if (NULL == p_wsmnp->p_wbuf_no_md && 0 == klb_list_size(p_wsmnp->p_wlist_no_md))
    {
        // 无数据可发送
        emws_socket_clear_flag_write(p_emws);
    }

    return send;
}
