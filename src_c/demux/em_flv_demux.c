#include "em_flv_demux.h"
#include "em_flv_parser.h"
#include "mem/klb_mem.h"
#include "log/klb_log.h"
#include "mnp/klb_mnp.h"
#include "em_util/em_buf.h"
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

typedef struct em_flv_demux_t_
{
    int             status;                 ///< 接收数据的状态
#define EM_FLV_DEMUX_STATUS_HEADER      0   ///< 接收FLV头
#define EM_FLV_DEMUX_STATUS_TAG         1   ///< 接收tag头
#define EM_FLV_DEMUX_STATUS_TAG_BODY    2   ///< 接收tag数据体

    em_flv_header_t header;
    em_flv_tag_t    tag;

    em_buf_t*       p_parser_buf;

    int             tag_body_spare;
    em_buf_t*       p_tag_body_buf;

    em_flv_avc_decoder_configuration_record_t   flv_adcr;
    bool                                        b_update_adcr;

    em_buf_t*       p_vps_buf;
    em_buf_t*       p_sps_buf;
    em_buf_t*       p_pps_buf;

    em_flv_audio_info_t aac_seq_head_info;  ///< AAC头信息
    bool                b_update_aac_seq_head;
}em_flv_demux_t;


static uint8_t s_em_flv_demux_nalu_h4[4] = { 0x0, 0x0, 0x0, 0x1 };


em_flv_demux_t* em_flv_demux_create()
{
    em_flv_demux_t* p_flv_demux = KLB_MALLOC(em_flv_demux_t, 1, 0);
    KLB_MEMSET(p_flv_demux, 0, sizeof(em_flv_demux_t));

    p_flv_demux->status = EM_FLV_DEMUX_STATUS_HEADER;
    p_flv_demux->p_parser_buf = em_buf_malloc_ref(EM_FLV_HEADER_SIZE0_LEN + EM_FLV_TAG_LEN);

    p_flv_demux->b_update_adcr = false;
    p_flv_demux->b_update_aac_seq_head = false;

    return p_flv_demux;
}

void em_flv_demux_destroy(em_flv_demux_t* p_flv_demux)
{
    assert(NULL != p_flv_demux);

    KLB_FREE_BY(p_flv_demux->p_sps_buf, em_buf_unref_next);
    KLB_FREE_BY(p_flv_demux->p_pps_buf, em_buf_unref_next);

    KLB_FREE_BY(p_flv_demux->p_parser_buf, em_buf_unref_next);
    KLB_FREE_BY(p_flv_demux->p_tag_body_buf, em_buf_unref_next);

    KLB_FREE(p_flv_demux);
}

static void em_flv_demux_update_vps(em_flv_demux_t* p_flv_demux, uint8_t* p_vps, uint16_t vps_len)
{
    if (NULL != p_flv_demux->p_vps_buf &&
        p_flv_demux->p_vps_buf->buf_len <= (vps_len + 4))
    {
        KLB_FREE_BY(p_flv_demux->p_vps_buf, em_buf_unref_next); // 缓存长度不足, 释放掉; 重新申请
        p_flv_demux->p_vps_buf = NULL;
    }

    if (NULL == p_flv_demux->p_vps_buf)
    {
        p_flv_demux->p_vps_buf = em_buf_malloc_ref(vps_len + 32);
    }

    memcpy(p_flv_demux->p_vps_buf->p_buf, s_em_flv_demux_nalu_h4, 4);
    memcpy(p_flv_demux->p_vps_buf->p_buf + 4, p_vps, vps_len);

    em_buf_set_pos(p_flv_demux->p_vps_buf, 0, vps_len + 4);
}

static void em_flv_demux_update_sps(em_flv_demux_t* p_flv_demux, uint8_t* p_sps, uint16_t sps_len)
{
    if (NULL != p_flv_demux->p_sps_buf &&
        p_flv_demux->p_sps_buf->buf_len <= (sps_len + 4))
    {
        KLB_FREE_BY(p_flv_demux->p_sps_buf, em_buf_unref_next); // 缓存长度不足, 释放掉; 重新申请
        p_flv_demux->p_sps_buf = NULL;
    }

    if (NULL == p_flv_demux->p_sps_buf)
    {
        p_flv_demux->p_sps_buf = em_buf_malloc_ref(sps_len + 32);
    }

    memcpy(p_flv_demux->p_sps_buf->p_buf, s_em_flv_demux_nalu_h4, 4);
    memcpy(p_flv_demux->p_sps_buf->p_buf + 4, p_sps, sps_len);

    em_buf_set_pos(p_flv_demux->p_sps_buf, 0, sps_len + 4);
}

static void em_flv_demux_update_pps(em_flv_demux_t* p_flv_demux, uint8_t* p_pps, uint16_t pps_len)
{
    if (NULL != p_flv_demux->p_pps_buf &&
        p_flv_demux->p_pps_buf->buf_len <= (pps_len + 4))
    {
        KLB_FREE_BY(p_flv_demux->p_pps_buf, em_buf_unref_next); // 缓存长度不足, 释放掉; 重新申请
        p_flv_demux->p_pps_buf = NULL;
    }

    if (NULL == p_flv_demux->p_pps_buf)
    {
        p_flv_demux->p_pps_buf = em_buf_malloc_ref(pps_len + 32);
    }

    memcpy(p_flv_demux->p_pps_buf->p_buf, s_em_flv_demux_nalu_h4, 4);
    memcpy(p_flv_demux->p_pps_buf->p_buf + 4, p_pps, pps_len);

    em_buf_set_pos(p_flv_demux->p_pps_buf, 0, pps_len + 4);
}

static int em_flv_demux_on_avc_sps_pps(em_flv_demux_t* p_flv_demux, em_flv_video_info_t* p_info)
{
    // h264,更新SPS/PPS
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
            em_flv_demux_update_sps(p_flv_demux, ptr, sps_len);
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
            em_flv_demux_update_pps(p_flv_demux, ptr, pps_len);
            b_update_sps = false;
        }

        //LOG("em flv parser pps len:[%d]", pps_len);
        ptr += pps_len;
    }

    return 0;
}

static int em_flv_demux_on_avc_vps_sps_pps_h265(em_flv_demux_t* p_flv_demux, em_flv_video_info_t* p_info)
{
    // h265,更新VPS/SPS/PPS
    uint8_t* ptr = p_info->p_sps_pps;
    int spare_len = p_info->sps_pps_len;

    bool b_update_vps = true, b_update_sps = true, b_update_pps = true;

    // VPS
    uint8_t num_of_vps = *ptr;                  ptr += 1;   // numOfSequenceParameterSets
    uint8_t vps_count = num_of_vps & 0x1F;
    for (int i = 0; i < vps_count; i++)
    {
        uint16_t vps_len = MNP_READ_BE16(ptr);  ptr += 2;   // sequenceParameterSetLength
        if (0 == vps_len)
        {
            continue;
        }

        if (b_update_vps)
        {
            em_flv_demux_update_vps(p_flv_demux, ptr, vps_len);
            b_update_vps = false;
        }

        //LOG("C em flv parser vps len:[%d]", vps_len);
        ptr += vps_len;
    }


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
            em_flv_demux_update_sps(p_flv_demux, ptr, sps_len);
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
            em_flv_demux_update_pps(p_flv_demux, ptr, pps_len);
            b_update_sps = false;
        }

        //LOG("em flv parser pps len:[%d]", pps_len);
        ptr += pps_len;
    }

    return 0;
}

static int em_flv_demux_on_avc_nalu(em_flv_demux_t* p_flv_demux, em_flv_video_info_t* p_info, klb_list_t* p_list_out)
{
    if (!p_flv_demux->b_update_adcr)
    {
        return 1; // 没有得到 sps/pps 信息前, 不接收nalu数据
    }

    uint8_t* ptr = p_info->p_nalu;
    int nalu_len = p_info->nalu_len;
    int length_size_minus_one = p_flv_demux->flv_adcr.length_size_minus_one;

    if (3 == length_size_minus_one || 4 == length_size_minus_one)
    {
        em_buf_t* p_frame = em_buf_malloc_ref(nalu_len + p_flv_demux->p_sps_buf->end + p_flv_demux->p_pps_buf->end + sizeof(klb_mnp_media_t) + 4 * (p_info->nalu_count + 4));
        p_frame->end = sizeof(klb_mnp_media_t);

        if (EM_FLV_AVC_KEY_FRAME == p_info->video.frame_type)
        {
            memcpy(p_frame->p_buf + p_frame->end, p_flv_demux->p_sps_buf->p_buf, p_flv_demux->p_sps_buf->end);
            p_frame->end += p_flv_demux->p_sps_buf->end;

            memcpy(p_frame->p_buf + p_frame->end, p_flv_demux->p_pps_buf->p_buf, p_flv_demux->p_pps_buf->end);
            p_frame->end += p_flv_demux->p_pps_buf->end;
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

            memcpy(p_frame->p_buf + p_frame->end, s_em_flv_demux_nalu_h4, 4);
            p_frame->end += 4;

            memcpy(p_frame->p_buf + p_frame->end, ptr, nalu_size);
            p_frame->end += nalu_size;

            ptr += nalu_size;
            offset += nalu_size + size_minus_one;
        }

        uint32_t ts = ((uint32_t)(p_flv_demux->tag.timestamp_extended) << 24) | p_flv_demux->tag.timestamp;

        // 填写其他信息
        klb_mnp_media_t* p_media = (klb_mnp_media_t*)p_frame->p_buf;
        p_media->size = p_frame->end;
        p_media->dtype = KLB_MNP_DTYPE_H264;
        p_media->chnn = 0;
        p_media->sidx = 0;
        p_media->time = (int64_t)(ts) * 1000;

        p_media->vtype = (EM_FLV_AVC_KEY_FRAME == p_info->video.frame_type) ? KLB_MNP_VTYPE_I : KLB_MNP_VTYPE_P;


        // 得到完整的一帧了
        //em_conn_manage_push_md(p_wsflv->p_conn_manage, EM_NET_WS_FLV, p_wsflv->name, p_frame);
        klb_list_push_tail(p_list_out, p_frame);
    }

    return 0;
}

static int em_flv_demux_on_avc_nalu_h265(em_flv_demux_t* p_flv_demux, em_flv_video_info_t* p_info, klb_list_t* p_list_out)
{
    // h265
    if (!p_flv_demux->b_update_adcr)
    {
        return 1; // 没有得到 vps/sps/pps 信息前, 不接收nalu数据
    }

    uint8_t* ptr = p_info->p_nalu;
    int nalu_len = p_info->nalu_len;
    int length_size_minus_one = p_flv_demux->flv_adcr.length_size_minus_one;

    if (3 == length_size_minus_one || 4 == length_size_minus_one)
    {
        em_buf_t* p_frame = em_buf_malloc_ref(nalu_len + p_flv_demux->p_sps_buf->end + p_flv_demux->p_pps_buf->end + sizeof(klb_mnp_media_t) + 4 * (p_info->nalu_count + 4));
        p_frame->end = sizeof(klb_mnp_media_t);

        if (EM_FLV_AVC_KEY_FRAME == p_info->video.frame_type)
        {
            memcpy(p_frame->p_buf + p_frame->end, p_flv_demux->p_vps_buf->p_buf, p_flv_demux->p_vps_buf->end);
            p_frame->end += p_flv_demux->p_vps_buf->end;

            memcpy(p_frame->p_buf + p_frame->end, p_flv_demux->p_sps_buf->p_buf, p_flv_demux->p_sps_buf->end);
            p_frame->end += p_flv_demux->p_sps_buf->end;

            memcpy(p_frame->p_buf + p_frame->end, p_flv_demux->p_pps_buf->p_buf, p_flv_demux->p_pps_buf->end);
            p_frame->end += p_flv_demux->p_pps_buf->end;
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

            memcpy(p_frame->p_buf + p_frame->end, s_em_flv_demux_nalu_h4, 4);
            p_frame->end += 4;

            memcpy(p_frame->p_buf + p_frame->end, ptr, nalu_size);
            p_frame->end += nalu_size;

            ptr += nalu_size;
            offset += nalu_size + size_minus_one;
        }

        uint32_t ts = ((uint32_t)(p_flv_demux->tag.timestamp_extended) << 24) | p_flv_demux->tag.timestamp;

        // 填写其他信息
        klb_mnp_media_t* p_media = (klb_mnp_media_t*)p_frame->p_buf;
        p_media->size = p_frame->end;
        p_media->dtype = KLB_MNP_DTYPE_H265;
        p_media->chnn = 0;
        p_media->sidx = 0;
        p_media->time = (int64_t)(ts) * 1000;

        p_media->vtype = (EM_FLV_AVC_KEY_FRAME == p_info->video.frame_type) ? KLB_MNP_VTYPE_I : KLB_MNP_VTYPE_P;


        // 得到完整的一帧了
        //em_conn_manage_push_md(p_wsflv->p_conn_manage, EM_NET_WS_FLV, p_wsflv->name, p_frame);
        klb_list_push_tail(p_list_out, p_frame);
    }

    return 0;
}

static int em_flv_demux_on_aac_raw(em_flv_demux_t* p_flv_demux, em_flv_audio_info_t* p_info, klb_list_t* p_list_out)
{
    if (!p_flv_demux->b_update_aac_seq_head)
    {
        return 1; // 没有获取AAC音频头, 不接收raw数据
    }

    uint8_t* p_raw = p_info->p_aac_raw;
    int raw_len = p_info->aac_raw_len;

    if (raw_len <= 0)
    {
        return 1;
    }

    em_buf_t* p_frame = em_buf_malloc_ref(raw_len + sizeof(klb_mnp_media_t));
    p_frame->end = sizeof(klb_mnp_media_t);

    // raw
    memcpy(p_frame->p_buf + p_frame->end, p_raw, raw_len);
    p_frame->end += raw_len;

    uint32_t ts = ((uint32_t)(p_flv_demux->tag.timestamp_extended) << 24) | p_flv_demux->tag.timestamp;

    // 填写其他信息
    klb_mnp_media_t* p_media = (klb_mnp_media_t*)p_frame->p_buf;
    p_media->size = p_frame->end;
    p_media->dtype = KLB_MNP_DTYPE_AAC;
    p_media->chnn = 0;
    p_media->sidx = 0;
    p_media->time = (int64_t)(ts) * 1000;

    p_media->tracks = p_flv_demux->aac_seq_head_info.aac_seq_head.channels; //1.单声道; 2.立体声
    p_media->bits_per_coded_sample = (EM_FLV_AUDIO_SAMPLES_8BIT == p_flv_demux->aac_seq_head_info.audio.sound_size) ? 1 : 2;
    p_media->samples = p_flv_demux->aac_seq_head_info.aac_seq_head.sampling_frequency;

    // 得到完整的一帧了
    klb_list_push_tail(p_list_out, p_frame);

    return 0;
}

static int em_flv_demux_on_tag(em_flv_demux_t* p_flv_demux, em_buf_t* p_buf, klb_list_t* p_list_out)
{
    em_flv_header_t* p_header = &p_flv_demux->header;
    em_flv_tag_t* p_tag = &p_flv_demux->tag;

    if (EM_FLV_TAG_VIDEO == p_tag->tag_type)
    {
        em_flv_video_info_t info = { 0 };

        int length_size_minus_one = p_flv_demux->b_update_adcr ? p_flv_demux->flv_adcr.length_size_minus_one : 0;
        int ret = em_flv_parser_tag_video(&info, (const char*)p_buf->p_buf, p_buf->end - 4, length_size_minus_one);
        if (0 == ret)
        {
            if (EM_FLV_CODEC_AVC == info.video.codec_id)
            {
                if (EM_FLV_AVC_SEQUENCE_HEADER == info.avc.avc_packet_type)
                {
                    p_flv_demux->flv_adcr = info.adcr;
                    p_flv_demux->b_update_adcr = true;

                    if (em_flv_demux_on_avc_sps_pps(p_flv_demux, &info) < 0)
                    {
                        return -1;
                    }
                }
                else if (EM_FLV_AVC_NALU == info.avc.avc_packet_type)
                {
                    if (em_flv_demux_on_avc_nalu(p_flv_demux, &info, p_list_out) < 0)
                    {
                        return -1;
                    }
                }
            }
            else if (EM_FLV_CODEC_AVC_H265 == info.video.codec_id)
            {
                if (EM_FLV_AVC_SEQUENCE_HEADER == info.avc.avc_packet_type)
                {
                    p_flv_demux->flv_adcr = info.adcr;
                    p_flv_demux->b_update_adcr = true;

                    if (em_flv_demux_on_avc_vps_sps_pps_h265(p_flv_demux, &info) < 0)
                    {
                        return -1;
                    }
                }
                else if (EM_FLV_AVC_NALU == info.avc.avc_packet_type)
                {
                    if (em_flv_demux_on_avc_nalu_h265(p_flv_demux, &info, p_list_out) < 0)
                    {
                        return -1;
                    }
                }
            }
        }
        else if (ret < 0)
        {
            return ret;
        }
    }
    else if (EM_FLV_TAG_AUDIO == p_tag->tag_type)
    {
        em_flv_audio_info_t audio_info = { 0 };

        int ret = em_flv_parser_tag_audio(&audio_info, (const char*)p_buf->p_buf, p_buf->end - 4);
        if (0 == ret)
        {
            if (EM_FLV_AUDIO_AAC == audio_info.audio.sound_format)
            {
                if (EM_FLV_AUDIO_AAC_SEQUENCE_HEADER == audio_info.aac_packet_type)
                {
                    p_flv_demux->aac_seq_head_info = audio_info;
                    p_flv_demux->b_update_aac_seq_head = true;
                }
                else if (EM_FLV_AUDIO_AAC_RAW == audio_info.aac_packet_type)
                {
                    if (em_flv_demux_on_aac_raw(p_flv_demux, &audio_info, p_list_out) < 0)
                    {
                        return -1;
                    }
                }
            } 
        }
        else if (ret < 0)
        {
            return ret;
        }
    }

    return 0;
}

int em_flv_demux_do(em_flv_demux_t* p_flv_demux, const char* p_data, int data_len, klb_list_t* p_list_out)
{
    assert(NULL != p_flv_demux);
    assert(NULL != p_data);
    assert(0 < data_len);
    assert(NULL != p_list_out);

    int ret_do = 0;

    em_buf_t* p_buf = p_flv_demux->p_parser_buf;

    while (0 < data_len)
    {
        if (EM_FLV_DEMUX_STATUS_HEADER == p_flv_demux->status)
        {
            int cp_len = MIN(EM_FLV_HEADER_SIZE0_LEN - p_buf->end, data_len);
            if (0 < cp_len)
            {
                memcpy(p_buf->p_buf + p_buf->end, p_data, cp_len);
                p_buf->end += cp_len;
                p_data += cp_len;
                data_len -= cp_len;
            }
            assert(p_buf->end <= EM_FLV_HEADER_SIZE0_LEN);

            int ret = em_flv_parser_header(&p_flv_demux->header, (const char*)p_buf->p_buf, p_buf->end);
            if (0 == ret)
            {
                p_buf->end = 0;
                p_flv_demux->status = EM_FLV_DEMUX_STATUS_TAG;
            }
            else if (ret < 0)
            {
                ret_do = ret; // 数据解析错误
                assert(false);
                break;
            }
            else
            {
                assert(0 == data_len);
            }
        }
        else if (EM_FLV_DEMUX_STATUS_TAG == p_flv_demux->status)
        {
            int cp_len = MIN(EM_FLV_TAG_LEN - p_buf->end, data_len);
            if (0 < cp_len)
            {
                memcpy(p_buf->p_buf + p_buf->end, p_data, cp_len);
                p_buf->end += cp_len;
                p_data += cp_len;
                data_len -= cp_len;
            }
            assert(p_buf->end <= EM_FLV_TAG_LEN);

            int ret = em_flv_parser_tag(&p_flv_demux->tag, (const char*)p_buf->p_buf, p_buf->end);
            if (0 == ret)
            {
                p_buf->end = 0;
                p_flv_demux->tag_body_spare = p_flv_demux->tag.data_size + 4;
                assert(NULL == p_flv_demux->p_tag_body_buf);
                p_flv_demux->p_tag_body_buf = em_buf_malloc_ref(p_flv_demux->tag_body_spare);
                p_flv_demux->status = EM_FLV_DEMUX_STATUS_TAG_BODY;
            }
            else if (ret < 0)
            {
                ret_do = ret; // 数据解析错误
                assert(false);
                break;
            }
            else
            {
                assert(0 == data_len);
            }
        }
        else
        {
            assert(EM_FLV_DEMUX_STATUS_TAG_BODY == p_flv_demux->status);

            int use_len = MIN(p_flv_demux->tag_body_spare, data_len);

            em_buf_t* p_body = p_flv_demux->p_tag_body_buf;
            if (0 < use_len)
            {
                memcpy(p_body->p_buf + p_body->end, p_data, use_len);
                p_body->end += use_len;

                p_data += use_len;
                data_len -= use_len;
                p_flv_demux->tag_body_spare -= use_len;
            }

            if (p_flv_demux->tag_body_spare <= 0)
            {
                // 一个tag数据接收完毕, 转入接收tag头
                assert(0 == p_flv_demux->tag_body_spare);

                if (em_flv_demux_on_tag(p_flv_demux, p_flv_demux->p_tag_body_buf, p_list_out) < 0)
                {
                    ret_do = -1;
                    break;
                }

                em_buf_unref_next(p_flv_demux->p_tag_body_buf);
                p_flv_demux->p_tag_body_buf = NULL;

                p_buf->end = 0;
                p_flv_demux->status = EM_FLV_DEMUX_STATUS_TAG;
            }
        }
    }

    return ret_do;
}
