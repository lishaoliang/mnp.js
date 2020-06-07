#include "em_ws_mnp.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"
#include "list/klb_list.h"
#include "em_util/em_buf.h"
#include "em_util/em_buf_mnp.h"
#include "mnp/klb_mnp.h"
#include "em_net/em_conn_manage_in.h"
#include <assert.h>

typedef struct emws_mnp_t_
{
    char                name[EM_NET_NAME_BUF];

    em_conn_manage_t*   p_conn_manage;
    emws_socket_t*      p_emws;

    klb_list_t*         p_wlist_no_md;
    em_buf_t*           p_wbuf_no_md;
    int                 pos_wbuf_no_md;

    struct
    {
        int             status;         ///< 接收数据的状态
#define EMWS_MNP_STATUS_HEADER      0   ///< 接收mnp头
#define EMWS_MNP_STATUS_BODY        1   ///< 接收mnp数据体

        em_buf_t*       p_parser_buf;   ///< 解析头部缓存
        klb_mnp_t       mnp_head;       ///< 当前mnp头

        int             body_spare;     ///< 还需要接收body的数据长度
        em_buf_t*       p_body_buf;     ///< body缓存
        em_buf_t*       p_frame;        ///< 完整数据帧
    };
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

    p_wsmnp->p_parser_buf = em_buf_malloc_ref(sizeof(klb_mnp_t));

    p_wsmnp->status = EMWS_MNP_STATUS_HEADER;

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
        em_buf_unref_next(p_tmp);
    }

    KLB_FREE_BY(p_wsmnp->p_parser_buf, em_buf_unref_next);

    KLB_FREE_BY(p_wsmnp->p_body_buf, em_buf_unref_next);
    KLB_FREE_BY(p_wsmnp->p_frame, em_buf_unref_next);

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

static int emws_mnp_on_frame(emws_mnp_t* p_wsmnp, uint8_t packtype, em_buf_t* p_frame)
{
    switch (packtype)
    {
    case KLB_MNP_HEART:
        break;
    case KLB_MNP_TXT:
        break;
    case KLB_MNP_BIN:
        break;
    case KLB_MNP_MEDIA:
        {
            em_buf_t* p_data = em_buf_mnp_join_md2(p_frame);
            if (NULL != p_data)
            {
                em_conn_manage_push_md(p_wsmnp->p_conn_manage, EM_NET_WS_MNP, p_wsmnp->name, p_data);
            }
        }
        break;
    default:
        break;
    }

    em_buf_unref_next(p_frame);
    return 0;
}

int emws_mnp_on_recv(emws_socket_t* p_emws, uint32_t now_ticks, const EmscriptenWebSocketMessageEvent* p_message)
{
    assert(NULL != p_emws);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emws->p_user1;
    emws_mnp_t* p_wsmnp = (emws_mnp_t*)p_emws->p_user2;
    assert(NULL != p_wsmnp);

    int recv = p_message->numBytes;
    //LOG("emws_mnp_on_recv:%d", recv);

    if (0 < recv && !p_message->isText)
    {
        // 只处理二进制, 仅认为: websocket做透明传输, 仅保证了字节流的次序
        // 这里需要再次分析协议数据, 不能依赖websocket的包具有某种结构
        // 原则上此方法可以处理具有分包信息的任何上层协议
        // 例如: WS-FLV, 甚至RTSP/RTMP等等
        // 服务端也仅需遵守websocket部分规范即可
        uint8_t* p_data = p_message->data;
        int data_len = recv;
        em_buf_t* p_buf = p_wsmnp->p_parser_buf;

        while (0 < data_len)
        {
            if (EMWS_MNP_STATUS_HEADER == p_wsmnp->status)
            {
                int cp_len = MIN(sizeof(klb_mnp_t) - p_buf->end, data_len);
                if (0 < cp_len)
                {
                    memcpy(p_buf->p_buf + p_buf->end, p_data, cp_len);

                    p_buf->end += cp_len;
                    p_data += cp_len;
                    data_len -= cp_len;
                }
                assert(p_buf->end <= sizeof(klb_mnp_t));

                if (sizeof(klb_mnp_t) <= p_buf->end)
                {
                    // mnp 头接收完毕
                    klb_mnp_t* p_mnp = p_buf->p_buf;
                    p_wsmnp->mnp_head = *p_mnp;

                    //LOG("recv: [%d,%d,%d]", p_mnp->opt, p_mnp->packtype, p_mnp->size);

                    // 转入接收body
                    p_buf->end = 0;
                    p_wsmnp->body_spare = p_mnp->size - sizeof(klb_mnp_t);
                    
                    assert(NULL == p_wsmnp->p_body_buf);
                    p_wsmnp->p_body_buf = em_buf_malloc_ref(p_mnp->size);
                    memcpy(p_wsmnp->p_body_buf->p_buf, p_mnp, sizeof(klb_mnp_t));
                    p_wsmnp->p_body_buf->end = sizeof(klb_mnp_t);

                    p_wsmnp->status = EMWS_MNP_STATUS_BODY;
                }
            }
            else
            {
                assert(EMWS_MNP_STATUS_BODY == p_wsmnp->status);
                int use_len = MIN(p_wsmnp->body_spare, data_len);

                if (0 < use_len)
                {
                    // 使用数据
                    memcpy(p_wsmnp->p_body_buf->p_buf + p_wsmnp->p_body_buf->end, p_data, use_len);
                    p_wsmnp->p_body_buf->end += use_len;

                    p_data += use_len;
                    data_len -= use_len;
                    p_wsmnp->body_spare -= use_len;
                }

                if (p_wsmnp->body_spare <= 0)
                {
                    assert(0 == p_wsmnp->body_spare);

                    if (KLB_MNP_FULL == p_wsmnp->mnp_head.opt)
                    {
                        // 完整包
                        assert(NULL == p_wsmnp->p_frame);
                        p_wsmnp->p_frame = em_buf_append(NULL, p_wsmnp->p_body_buf);

                        emws_mnp_on_frame(p_wsmnp, p_wsmnp->mnp_head.packtype, p_wsmnp->p_frame);
                        p_wsmnp->p_frame = NULL;
                    }
                    else if (KLB_MNP_BEGIN == p_wsmnp->mnp_head.opt)
                    {
                        // 分包
                        assert(NULL == p_wsmnp->p_frame);
                        p_wsmnp->p_frame = em_buf_append(NULL, p_wsmnp->p_body_buf);
                    }
                    else if (KLB_MNP_CONTINUE == p_wsmnp->mnp_head.opt)
                    {
                        p_wsmnp->p_frame = em_buf_append(p_wsmnp->p_frame, p_wsmnp->p_body_buf);
                    }
                    else if (KLB_MNP_END == p_wsmnp->mnp_head.opt)
                    {
                        p_wsmnp->p_frame = em_buf_append(p_wsmnp->p_frame, p_wsmnp->p_body_buf);

                        emws_mnp_on_frame(p_wsmnp, p_wsmnp->mnp_head.packtype, p_wsmnp->p_frame);
                        p_wsmnp->p_frame = NULL;
                    }
                    else
                    {
                        assert(false);
                        em_buf_unref_next(p_wsmnp->p_body_buf);
                    }

                    // 转入接收header
                    p_wsmnp->p_body_buf = NULL;
                    p_wsmnp->status = EMWS_MNP_STATUS_HEADER;
                }
            }
        }
    }

    return recv;
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
