#include "em_mnp_demux.h"
#include "mem/klb_mem.h"
#include "log/klb_log.h"
#include "mnp/klb_mnp.h"
#include "em_util/em_buf.h"
#include "em_util/em_buf_mnp.h"
#include <assert.h>

typedef struct em_mnp_demux_t_ 
{
    int             status;             ///< 接收数据的状态
#define EM_MNP_DEMUX_STATUS_HEADER  0   ///< 接收mnp头
#define EM_MNP_DEMUX_STATUS_BODY    1   ///< 接收mnp数据体

    em_buf_t*       p_parser_buf;       ///< 解析头部缓存
    klb_mnp_t       mnp_head;           ///< 当前mnp头

    int             body_spare;         ///< 还需要接收body的数据长度
    em_buf_t*       p_body_buf;         ///< body缓存
    em_buf_t*       p_frame;            ///< 完整数据帧
}em_mnp_demux_t;


em_mnp_demux_t* em_mnp_demux_create()
{
    em_mnp_demux_t* p_mnp_demux = KLB_MALLOC(em_mnp_demux_t, 1, 0);
    KLB_MEMSET(p_mnp_demux, 0, sizeof(em_mnp_demux_t));

    p_mnp_demux->p_parser_buf = em_buf_malloc_ref(sizeof(klb_mnp_t));

    p_mnp_demux->status = EM_MNP_DEMUX_STATUS_HEADER;

    return p_mnp_demux;
}

void em_mnp_demux_destroy(em_mnp_demux_t* p_mnp_demux)
{
    assert(NULL != p_mnp_demux);

    KLB_FREE_BY(p_mnp_demux->p_parser_buf, em_buf_unref_next);

    KLB_FREE_BY(p_mnp_demux->p_body_buf, em_buf_unref_next);
    KLB_FREE_BY(p_mnp_demux->p_frame, em_buf_unref_next);

    KLB_FREE(p_mnp_demux);
}


static int emws_mnp_on_frame(em_mnp_demux_t* p_mnp_demux, uint8_t packtype, em_buf_t* p_frame, klb_list_t* p_list_out)
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
            klb_list_push_tail(p_list_out, p_data);
        }
    }
    break;
    default:
        break;
    }

    em_buf_unref_next(p_frame);
    return 0;
}

int em_mnp_demux_do(em_mnp_demux_t* p_mnp_demux, const char* p_data, int data_len, klb_list_t* p_list_out)
{
    em_buf_t* p_buf = p_mnp_demux->p_parser_buf;

    while (0 < data_len)
    {
        if (EM_MNP_DEMUX_STATUS_HEADER == p_mnp_demux->status)
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
                klb_mnp_t* p_mnp = (klb_mnp_t*)p_buf->p_buf;
                p_mnp_demux->mnp_head = *p_mnp;

                //LOG("recv: [%d,%d,%d]", p_mnp->opt, p_mnp->packtype, p_mnp->size);

                // 转入接收body
                p_buf->end = 0;
                p_mnp_demux->body_spare = p_mnp->size - sizeof(klb_mnp_t);

                assert(NULL == p_mnp_demux->p_body_buf);
                p_mnp_demux->p_body_buf = em_buf_malloc_ref(p_mnp->size);
                memcpy(p_mnp_demux->p_body_buf->p_buf, p_mnp, sizeof(klb_mnp_t));
                p_mnp_demux->p_body_buf->end = sizeof(klb_mnp_t);

                p_mnp_demux->status = EM_MNP_DEMUX_STATUS_BODY;
            }
        }
        else
        {
            assert(EM_MNP_DEMUX_STATUS_BODY == p_mnp_demux->status);
            int use_len = MIN(p_mnp_demux->body_spare, data_len);

            if (0 < use_len)
            {
                // 使用数据
                memcpy(p_mnp_demux->p_body_buf->p_buf + p_mnp_demux->p_body_buf->end, p_data, use_len);
                p_mnp_demux->p_body_buf->end += use_len;

                p_data += use_len;
                data_len -= use_len;
                p_mnp_demux->body_spare -= use_len;
            }

            if (p_mnp_demux->body_spare <= 0)
            {
                assert(0 == p_mnp_demux->body_spare);

                if (KLB_MNP_FULL == p_mnp_demux->mnp_head.opt)
                {
                    // 完整包
                    assert(NULL == p_mnp_demux->p_frame);
                    p_mnp_demux->p_frame = em_buf_append(NULL, p_mnp_demux->p_body_buf);

                    emws_mnp_on_frame(p_mnp_demux, p_mnp_demux->mnp_head.packtype, p_mnp_demux->p_frame, p_list_out);
                    p_mnp_demux->p_frame = NULL;
                }
                else if (KLB_MNP_BEGIN == p_mnp_demux->mnp_head.opt)
                {
                    // 分包
                    assert(NULL == p_mnp_demux->p_frame);
                    p_mnp_demux->p_frame = em_buf_append(NULL, p_mnp_demux->p_body_buf);
                }
                else if (KLB_MNP_CONTINUE == p_mnp_demux->mnp_head.opt)
                {
                    p_mnp_demux->p_frame = em_buf_append(p_mnp_demux->p_frame, p_mnp_demux->p_body_buf);
                }
                else if (KLB_MNP_END == p_mnp_demux->mnp_head.opt)
                {
                    p_mnp_demux->p_frame = em_buf_append(p_mnp_demux->p_frame, p_mnp_demux->p_body_buf);

                    emws_mnp_on_frame(p_mnp_demux, p_mnp_demux->mnp_head.packtype, p_mnp_demux->p_frame, p_list_out);
                    p_mnp_demux->p_frame = NULL;
                }
                else
                {
                    assert(false);
                    em_buf_unref_next(p_mnp_demux->p_body_buf);
                }

                // 转入接收header
                p_mnp_demux->p_body_buf = NULL;
                p_mnp_demux->status = EM_MNP_DEMUX_STATUS_HEADER;
            }
        }
    }

    return 0;
}
