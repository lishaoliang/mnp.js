#include "em_ws_flv.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"
#include "list/klb_list.h"
#include "em_util/em_buf.h"
#include "em_util/em_buf_mnp.h"
#include "mnp/klb_mnp.h"
#include "em_net/em_conn_manage_in.h"
#include "em_flv_parser.h"
#include <assert.h>


#ifndef MNP_READ_BE16
#define MNP_READ_BE16(x)                            \
        ((((const uint8_t*)(x))[0] << 8) |          \
        ((const uint8_t*)(x))[1])
#endif


#ifndef MNP_READ_BE32
#define MNP_READ_BE32(x)                            \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) |   \
              (((const uint8_t*)(x))[1] << 16) |    \
              (((const uint8_t*)(x))[2] <<  8) |    \
              ((const uint8_t*)(x))[3])
#endif


typedef struct emws_flv_t_
{
    char                name[EM_NET_NAME_BUF];

    em_conn_manage_t*   p_conn_manage;
    emws_socket_t*      p_emws;

    struct
    {
        int             status;         ///< 接收数据的状态
#define EMWS_FLV_STATUS_HEADER      0   ///< 接收FLV头
#define EMWS_FLV_STATUS_TAG         1   ///< 接收tag头
#define EMWS_FLV_STATUS_TAG_BODY    2   ///< 接收tag数据体

        em_flv_header_t header;
        em_flv_tag_t    tag;

        em_buf_t*       p_parser_buf;

        int             tag_body_spare;
        em_buf_t*       p_tag_body_buf;

        em_flv_avc_decoder_configuration_record_t   flv_adcr;
        bool                                        b_update_adcr;

        em_buf_t*       p_sps_buf;
        em_buf_t*       p_pps_buf;
    };

}emws_flv_t;



static uint8_t s_emws_flv_nalu_h4[4] = {0x0, 0x0, 0x0, 0x1};


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


    p_wsflv->status = EMWS_FLV_STATUS_HEADER;
    p_wsflv->p_parser_buf = em_buf_malloc_ref(EM_FVL_HEADER_SIZE0_LEN + EM_FVL_TAG_LEN);

    p_wsflv->b_update_adcr = false;

    return p_wsflv;
}

void emws_flv_destroy(void* ptr)
{
    emws_flv_t* p_wsflv = (emws_flv_t*)ptr;
    assert(NULL != p_wsflv);

    KLB_FREE_BY(p_wsflv->p_sps_buf, em_buf_unref_next);
    KLB_FREE_BY(p_wsflv->p_pps_buf, em_buf_unref_next);

    KLB_FREE_BY(p_wsflv->p_parser_buf, em_buf_unref_next);
    KLB_FREE_BY(p_wsflv->p_tag_body_buf, em_buf_unref_next);

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

static void emws_flv_update_sps(emws_flv_t* p_wsflv, uint8_t* p_sps, uint16_t sps_len)
{
    if (NULL != p_wsflv->p_sps_buf && 
        p_wsflv->p_sps_buf->buf_len <= (sps_len + 4))
    {
        KLB_FREE_BY(p_wsflv->p_sps_buf, em_buf_unref_next); // 缓存长度不足, 释放掉; 重新申请
        p_wsflv->p_sps_buf = NULL;
    }

    if (NULL == p_wsflv->p_sps_buf)
    {
        p_wsflv->p_sps_buf = em_buf_malloc_ref(sps_len + 32);
    }

    memcpy(p_wsflv->p_sps_buf->p_buf, s_emws_flv_nalu_h4, 4);
    memcpy(p_wsflv->p_sps_buf->p_buf + 4, p_sps, sps_len);

    em_buf_set_pos(p_wsflv->p_sps_buf, 0, sps_len + 4);
}

static void emws_flv_update_pps(emws_flv_t* p_wsflv, uint8_t* p_pps, uint16_t pps_len)
{
    if (NULL != p_wsflv->p_pps_buf &&
        p_wsflv->p_pps_buf->buf_len <= (pps_len + 4))
    {
        KLB_FREE_BY(p_wsflv->p_pps_buf, em_buf_unref_next); // 缓存长度不足, 释放掉; 重新申请
        p_wsflv->p_pps_buf = NULL;
    }

    if (NULL == p_wsflv->p_pps_buf)
    {
        p_wsflv->p_pps_buf = em_buf_malloc_ref(pps_len + 32);
    }

    memcpy(p_wsflv->p_pps_buf->p_buf, s_emws_flv_nalu_h4, 4);
    memcpy(p_wsflv->p_pps_buf->p_buf + 4, p_pps, pps_len);

    em_buf_set_pos(p_wsflv->p_pps_buf, 0, pps_len + 4);
}

static int emws_flv_on_avc_sps_pps(emws_flv_t* p_wsflv, em_flv_video_info_t* p_info)
{
    // 更新SPS/PPS
    uint8_t* ptr = p_info->p_sps_pps;
    int spare_len = p_info->sps_pps_len;

    bool b_update_sps = true, b_update_pps = true;

    // SPS
    uint8_t num_of_sps = *ptr;                  ptr += 1;   // numOfSequenceParameterSets
    uint8_t sps_count = num_of_sps & 0x1F;
    for (int i = 0; i < sps_count; i++)
    {
        uint16_t sps_len = MNP_READ_BE16(ptr);  ptr += 2;   // sequenceParameterSetLength
        if (0 == sps_len)
        {
            continue;
        }

        if (b_update_sps)
        {
            emws_flv_update_sps(p_wsflv, ptr, sps_len);
            b_update_sps = false;
        }

        //LOG("em flv parser sps len:[%d]", sps_len);
        ptr += sps_len;
    }

    // PPS
    uint8_t pps_count = *ptr;                   ptr += 1;   // numOfPictureParameterSets
    for (int i = 0; i < pps_count; i++)
    {
        uint16_t pps_len = MNP_READ_BE16(ptr);  ptr += 2;   // pictureParameterSetLength
        if (0 == pps_len)
        {
            continue;
        }

        if (b_update_pps)
        {
            emws_flv_update_pps(p_wsflv, ptr, pps_len);
            b_update_sps = false;
        }

        //LOG("em flv parser pps len:[%d]", pps_len);
        ptr += pps_len;
    }

    return 0;
}

static int emws_flv_on_avc_nalu(emws_flv_t* p_wsflv, em_flv_video_info_t* p_info)
{
    if (!p_wsflv->b_update_adcr)
    {
        return 1; // 没有得到 sps/pps 信息前, 不接收nalu数据
    }   

    uint8_t* ptr = p_info->p_nalu;
    int nalu_len = p_info->nalu_len;
    int length_size_minus_one = p_wsflv->flv_adcr.length_size_minus_one;

    if (3 == length_size_minus_one || 4 == length_size_minus_one)
    {
        em_buf_t* p_frame = em_buf_malloc_ref(nalu_len + p_wsflv->p_sps_buf->end + p_wsflv->p_pps_buf->end + sizeof(klb_mnp_md_t) + 4 * (p_info->nalu_count + 4));
        p_frame->end = sizeof(klb_mnp_md_t);

        if (EM_FVL_AVC_KEY_FRAME == p_info->video.frame_type)
        {
            memcpy(p_frame->p_buf + p_frame->end, p_wsflv->p_sps_buf->p_buf, p_wsflv->p_sps_buf->end);
            p_frame->end += p_wsflv->p_sps_buf->end;

            memcpy(p_frame->p_buf + p_frame->end, p_wsflv->p_pps_buf->p_buf, p_wsflv->p_pps_buf->end);
            p_frame->end += p_wsflv->p_pps_buf->end;
        }

        // NALU
        int offset = 0;
        int size_minus_one = length_size_minus_one; //

        while (offset + 4 < nalu_len)
        {
            uint32_t nalu_size = MNP_READ_BE32(ptr);
            if (3 == size_minus_one)
            {
                nalu_size = nalu_size >> 8;
                ptr += 3;
            }
            else
            {
                ptr += 4;
            }

            if (nalu_len <= nalu_size)
            {
                assert(false);
                return -1;
            }
            
            memcpy(p_frame->p_buf + p_frame->end, s_emws_flv_nalu_h4, 4);
            p_frame->end += 4;

            memcpy(p_frame->p_buf + p_frame->end, ptr, nalu_size);
            p_frame->end += nalu_size;

            ptr += nalu_size;
            offset += nalu_size + size_minus_one;
        }

        // 填写其他信息
        klb_mnp_md_t* p_media = p_frame->p_buf;
        p_media->size = p_frame->end;
        p_media->dtype = KLB_MNP_DTYPE_H264;
        p_media->chnn = 0;
        p_media->sidx = 0;
        p_media->time = p_wsflv->tag.timestamp;

        p_media->vtype = (EM_FVL_AVC_KEY_FRAME == p_info->video.frame_type) ? KLB_MNP_VTYPE_I : KLB_MNP_VTYPE_P;


        // 得到完整的一帧了
        em_conn_manage_push_md(p_wsflv->p_conn_manage, EM_NET_WS_FLV, p_wsflv->name, p_frame);
    }

    return 0;
}

static int emws_flv_on_tag(emws_flv_t* p_wsflv, em_buf_t* p_buf)
{
    em_flv_header_t* p_header = &p_wsflv->header;
    em_flv_tag_t* p_tag = &p_wsflv->tag;

    if (EM_FVL_TAG_VIDEO == p_tag->tag_type)
    {
        em_flv_video_info_t info = { 0 };

        int length_size_minus_one = p_wsflv->b_update_adcr ? p_wsflv->flv_adcr.length_size_minus_one : 0;
        int ret = em_flv_parser_tag_video(&info, p_buf->p_buf, p_buf->end - 4, length_size_minus_one);
        if (0 == ret)
        {
            if (EM_FVL_CODEC_AVC == info.video.codec_id)
            {
                if (EM_FVL_AVC_SEQUENCE_HEADER == info.avc.avc_packet_type)
                {
                    p_wsflv->flv_adcr = info.adcr;
                    p_wsflv->b_update_adcr = true;

                    emws_flv_on_avc_sps_pps(p_wsflv, &info);
                }
                else if (EM_FVL_AVC_NALU == info.avc.avc_packet_type)
                {
                    emws_flv_on_avc_nalu(p_wsflv, &info);
                }
            }
        }
    }

    return 0;
}

int emws_flv_on_recv(emws_socket_t* p_emws, uint32_t now_ticks, const EmscriptenWebSocketMessageEvent* p_message)
{
    assert(NULL != p_emws);
    em_conn_env_t* p_env = (em_conn_env_t*)p_emws->p_user1;
    emws_flv_t* p_wsflv = (emws_flv_t*)p_emws->p_user2;
    assert(NULL != p_wsflv);

    int recv = p_message->numBytes;
    //LOG("emws_flv_on_recv:%d", recv);

    if (0 < recv && !p_message->isText)
    {
        // 只处理二进制
        uint8_t* p_data = p_message->data;
        int data_len = recv;
        em_buf_t* p_buf = p_wsflv->p_parser_buf;

        while (0 < data_len)
        {
            if (EMWS_FLV_STATUS_HEADER == p_wsflv->status)
            {
                int cp_len = MIN(EM_FVL_HEADER_SIZE0_LEN - p_buf->end, data_len);
                if (0 < cp_len)
                {
                    memcpy(p_buf->p_buf + p_buf->end, p_data, cp_len);
                    p_buf->end += cp_len;
                    p_data += cp_len;
                    data_len -= cp_len;
                }
                assert(p_buf->end <= EM_FVL_HEADER_SIZE0_LEN);

                int ret = em_flv_parser_header(&p_wsflv->header, p_buf->p_buf, p_buf->end);
                if (0 == ret)
                {
                    p_buf->end = 0;
                    p_wsflv->status = EMWS_FLV_STATUS_TAG;
                }
                else if (ret < 0)
                {
                    assert(false);
                }
                else
                {
                    assert(0 == data_len);
                }
            }
            else if (EMWS_FLV_STATUS_TAG == p_wsflv->status)
            {
                int cp_len = MIN(EM_FVL_TAG_LEN - p_buf->end, data_len);
                if (0 < cp_len)
                {
                    memcpy(p_buf->p_buf + p_buf->end, p_data, cp_len);
                    p_buf->end += cp_len;
                    p_data += cp_len;
                    data_len -= cp_len;
                }
                assert(p_buf->end <= EM_FVL_TAG_LEN);

                int ret = em_flv_parser_tag(&p_wsflv->tag, p_buf->p_buf, p_buf->end);
                if (0 == ret)
                {
                    p_buf->end = 0;
                    p_wsflv->tag_body_spare = p_wsflv->tag.data_size + 4;
                    assert(NULL == p_wsflv->p_tag_body_buf);
                    p_wsflv->p_tag_body_buf = em_buf_malloc_ref(p_wsflv->tag_body_spare);
                    p_wsflv->status = EMWS_FLV_STATUS_TAG_BODY;
                }
                else if (ret < 0)
                {
                    assert(false);
                }
                else
                {
                    assert(0 == data_len);
                }
            }
            else
            {
                assert(EMWS_FLV_STATUS_TAG_BODY == p_wsflv->status);

                int use_len = MIN(p_wsflv->tag_body_spare, data_len);

                em_buf_t* p_body = p_wsflv->p_tag_body_buf;
                if (0 < use_len)
                {
                    memcpy(p_body->p_buf + p_body->end, p_data, use_len);
                    p_body->end += use_len;
                    
                    p_data += use_len;
                    data_len -= use_len;
                    p_wsflv->tag_body_spare -= use_len;
                }

                if (p_wsflv->tag_body_spare <= 0)
                {
                    // 一个tag数据接收完毕, 转入接收tag头
                    assert(0 == p_wsflv->tag_body_spare);

                    emws_flv_on_tag(p_wsflv, p_wsflv->p_tag_body_buf);

                    em_buf_unref_next(p_wsflv->p_tag_body_buf);
                    p_wsflv->p_tag_body_buf = NULL;

                    p_buf->end = 0;
                    p_wsflv->status = EMWS_FLV_STATUS_TAG;
                }
            }
        }
    }

    return recv;
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
