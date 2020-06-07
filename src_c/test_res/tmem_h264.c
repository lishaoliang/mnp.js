#include "tmem_h264.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>


#define TMEM_FRAME_STEP     1000
#define TMEM_H264_MALLOC    malloc
#define TMEM_H264_FREE      free

typedef struct tmem_frame_t_
{
    int             type;    // tmem_h264_type_e
    int             len;
    char*           p_frame;
    tmem_h264_key_t h264_key;
}tmem_frame_t;

typedef struct tmem_h264_t_
{
    int             data_len;
    char*           p_data;

    int             frame_max;
    int             frame_num;

    tmem_frame_t*   p_frame;

    int             idx_auto;
}tmem_h264_t;


typedef enum tmem_h264_e_
{
    TMEM_H264_SPS   = 0x67,
    TMEM_H264_PPS   = 0x68,
    TMEM_H264_I     = 0x65,

    TMEM_H264_P61   = 0x61,
    TMEM_H264_P41   = 0x41
}tmem_h264_e;

#define H264_IS_TYPE(V_H264, NAL_TYPE) ((V_H264 & 0x1F) == (NAL_TYPE & 0x1F))

static int h264_type(unsigned char value)
{
    if (H264_IS_TYPE(value, TMEM_H264_SPS))
    {
        return TMEM_H264_SPS;
    }
    else if (H264_IS_TYPE(value, TMEM_H264_PPS))
    {
        return TMEM_H264_PPS;
    }
    else if (H264_IS_TYPE(value, TMEM_H264_I))
    {
        return TMEM_H264_I;
    }
    else if (H264_IS_TYPE(value, TMEM_H264_P61))
    {
        return TMEM_H264_P61;
    }
    else if (H264_IS_TYPE(value, TMEM_H264_P41))
    {
        return TMEM_H264_P41;
    }

    return 0;
}

static int scan_frame(unsigned char* p_h264, int h264_len, int* p_frame_pos, int* p_frame_len, int* p_frame_type)
{
    static unsigned char nal_h4[4] = { 0x00, 0x00, 0x00, 0x01 };
    static unsigned char nal_h3[3] = { 0x00, 0x00, 0x01 };

    int frame_pos = -1;
    int frame_len = 0;
    int frame_type = TMEM_H264_FRAME_B;
    char bit[4] = { 0 }; // [sps, pps, I, P61/P41]

    int pos = 0;
    while (pos + 4 < h264_len)
    {
        if (0 == memcmp(p_h264 + pos, nal_h3, 3))
        {
            int type = h264_type(*(p_h264 + pos + 3));
            //printf("v=%x, t=%x\n", *(p_h264 + pos + 3), type);

            int nal_pos = pos;
            if ((0 < nal_pos) && (0 == memcmp(p_h264 + nal_pos - 1, nal_h4, 4)))
            {
                nal_pos--;
            }

            if ((0 != bit[2]) || (0 != bit[3]))
            {
                if (0 <= frame_pos)
                    frame_len = nal_pos - frame_pos;

                break;
            }

            if (TMEM_H264_SPS == type)
            {
                bit[0] = 1;
            }
            else if (TMEM_H264_PPS == type)
            {
                bit[1] = 1;
            }
            else if (TMEM_H264_I == type)
            {
                bit[2] = 1;
                frame_type = TMEM_H264_FRAME_I;
            }
            else if ((TMEM_H264_P61 == type) ||
                (TMEM_H264_P41 == type))
            {
                bit[3] = 1;
            }
            else
            {

            }

            if (frame_pos < 0)
            {
                frame_pos = nal_pos;
            }

            pos += 3;
        }
        else
        {
            pos += 1;
        }
    }

    // 返回已经扫描的字节
    if ((0 <= frame_pos) &&
        ((0 != bit[2]) || (0 != bit[3])))
    {
        // 找到数据帧
        if (0 == frame_len)
        {
            frame_len = h264_len - frame_pos;
        }

        *p_frame_pos = frame_pos;
        *p_frame_len = frame_len;
        *p_frame_type = frame_type;

        return frame_pos + frame_len; // 返回数据偏移
    }
    else
    {
        *p_frame_pos = 0;
        *p_frame_len = 0;
        *p_frame_type = 0;

        return h264_len; // 这段数据没有帧
    }
}

static int scan_key_frame(tmem_frame_t* p_frame)
{
    static unsigned char nal_h4[4] = { 0x00, 0x00, 0x00, 0x01 };
    static unsigned char nal_h3[3] = { 0x00, 0x00, 0x01 };

    char* p_h264 = p_frame->p_frame;
    int h264_len = p_frame->len;

    char bit[4] = { 0 }; // [sps, pps, I, P61/P41]

    int last_bit = -1;
    int last_pos = 0;
    int pos = 0;
    while (pos + 4 < h264_len)
    {
        if (0 == memcmp(p_h264 + pos, nal_h3, 3))
        {
            int type = h264_type(*(p_h264 + pos + 3));
            //printf("v=%x, t=%x\n", *(p_h264 + pos + 3), type);

            int nal_pos = pos;
            if ((0 < nal_pos) && (0 == memcmp(p_h264 + nal_pos - 1, nal_h4, 4)))
            {
                nal_pos--;
            }

            if (TMEM_H264_SPS == type)
            {
                assert(0 == bit[0]);

                bit[0] = 1;

                p_frame->h264_key.p_sps = p_frame->p_frame + nal_pos;

                if (0 <= last_bit)
                {
                    if (0 == last_bit) p_frame->h264_key.sps_len = nal_pos - last_pos;
                    if (1 == last_bit) p_frame->h264_key.pps_len = nal_pos - last_pos;
                    if (2 == last_bit) p_frame->h264_key.i_len = nal_pos - last_pos;
                }

                last_bit = 0;
                last_pos = nal_pos;
            }
            else if (TMEM_H264_PPS == type)
            {
                assert(0 == bit[1]);

                bit[1] = 1;

                p_frame->h264_key.p_pps = p_frame->p_frame + nal_pos;

                if (0 <= last_bit)
                {
                    if (0 == last_bit) p_frame->h264_key.sps_len = nal_pos - last_pos;
                    if (1 == last_bit) p_frame->h264_key.pps_len = nal_pos - last_pos;
                    if (2 == last_bit) p_frame->h264_key.i_len = nal_pos - last_pos;
                }

                last_bit = 1;
                last_pos = nal_pos;
            }
            else if (TMEM_H264_I == type)
            {
                assert(0 == bit[2]);

                bit[2] = 1;

                p_frame->h264_key.p_i = p_frame->p_frame + nal_pos;

                if (0 <= last_bit)
                {
                    if (0 == last_bit) p_frame->h264_key.sps_len = nal_pos - last_pos;
                    if (1 == last_bit) p_frame->h264_key.pps_len = nal_pos - last_pos;
                    if (2 == last_bit) p_frame->h264_key.i_len = nal_pos - last_pos;
                }

                last_bit = 2;
                last_pos = nal_pos;
            }
            else if ((TMEM_H264_P61 == type) ||
                (TMEM_H264_P41 == type))
            {
                assert(0 == bit[3]);
                assert(false);

                bit[3] = 1;

                if (0 <= last_bit)
                {
                    if (0 == last_bit) p_frame->h264_key.sps_len = nal_pos - last_pos;
                    if (1 == last_bit) p_frame->h264_key.pps_len = nal_pos - last_pos;
                    if (2 == last_bit) p_frame->h264_key.i_len = nal_pos - last_pos;
                }

                last_bit = 3;
                last_pos = nal_pos;
            }
            else
            {
                if (0 <= last_bit)
                {
                    if (0 == last_bit) p_frame->h264_key.sps_len = nal_pos - last_pos;
                    if (1 == last_bit) p_frame->h264_key.pps_len = nal_pos - last_pos;
                    if (2 == last_bit) p_frame->h264_key.i_len = nal_pos - last_pos;
                }

                last_bit = -1;
            }

            pos += 3;
        }
        else
        {
            pos += 1;
        }
    }

    if (0 <= last_bit)
    {
        if (0 == last_bit) p_frame->h264_key.sps_len = h264_len - last_pos;
        if (1 == last_bit) p_frame->h264_key.pps_len = h264_len - last_pos;
        if (2 == last_bit) p_frame->h264_key.i_len = h264_len - last_pos;
    }

    assert(0 < p_frame->h264_key.sps_len);
    assert(0 < p_frame->h264_key.pps_len);
    assert(0 < p_frame->h264_key.i_len);
    assert(NULL != p_frame->h264_key.p_sps);
    assert(NULL != p_frame->h264_key.p_pps);
    assert(NULL != p_frame->h264_key.p_i);

    p_frame->h264_key.len = p_frame->h264_key.sps_len + p_frame->h264_key.pps_len + p_frame->h264_key.i_len;

    return 0;
}

static void h264_add_frame(tmem_h264_t* p_tmem_h264, char* p_frame, int frame_len, int frame_type)
{
    assert(NULL != p_tmem_h264);

    if (NULL == p_tmem_h264->p_frame)
    {
        assert(0 == p_tmem_h264->frame_max);
        assert(0 == p_tmem_h264->frame_num);

        p_tmem_h264->frame_max = TMEM_FRAME_STEP;
        p_tmem_h264->p_frame = (tmem_frame_t*)TMEM_H264_MALLOC(sizeof(tmem_frame_t) * p_tmem_h264->frame_max);
    }

    if (p_tmem_h264->frame_max < (p_tmem_h264->frame_num + 1))
    {
        assert(NULL != p_tmem_h264->p_frame);
        int frame_max = p_tmem_h264->frame_max + TMEM_FRAME_STEP;
        tmem_frame_t* p_tmp_frame = (tmem_frame_t*)TMEM_H264_MALLOC(sizeof(tmem_frame_t) * frame_max);

        memcpy(p_tmp_frame, p_tmem_h264->p_frame, sizeof(tmem_frame_t) * p_tmem_h264->frame_num);
        
        TMEM_H264_FREE(p_tmem_h264->p_frame);
        p_tmem_h264->p_frame = p_tmp_frame;
        p_tmem_h264->frame_max = frame_max;
    }


    int idx = p_tmem_h264->frame_num;
    p_tmem_h264->p_frame[idx].p_frame = p_frame;
    p_tmem_h264->p_frame[idx].len = frame_len;
    p_tmem_h264->p_frame[idx].type = frame_type;

    if (TMEM_H264_FRAME_I == frame_type)
    {
        scan_key_frame(&p_tmem_h264->p_frame[idx]);
    }
    else
    {
        memset(&p_tmem_h264->p_frame[idx].h264_key, 0, sizeof(tmem_h264_key_t));
    }

    p_tmem_h264->frame_num++;

    assert(p_tmem_h264->frame_num <= p_tmem_h264->frame_max);
}

tmem_h264_t* tmem_h264_open(const char* p_h264, int h264_len)
{
    tmem_h264_t* p_tmem_h264 = (tmem_h264_t*)TMEM_H264_MALLOC(sizeof(tmem_h264_t));
    memset(p_tmem_h264, 0, sizeof(tmem_h264_t));

    p_tmem_h264->p_data = (char*)p_h264;
    p_tmem_h264->data_len = h264_len;

    //////////////////////////////////////////////////////////////////////////
    // 分析数据
    unsigned char* ptr = (unsigned char*)p_h264;
    int data_len = h264_len;

    int pos = 0;
    while (pos < data_len)
    {
        int frame_pos = 0, frame_len = 0, frame_type = 0;
        int offset = scan_frame(ptr + pos, data_len - pos, &frame_pos, &frame_len, &frame_type);

        if (0 < frame_len)
        {
            h264_add_frame(p_tmem_h264, (char*)ptr + pos + frame_pos, frame_len, frame_type);
        }

        pos += offset;
    }

    return p_tmem_h264;
}

void tmem_h264_close(tmem_h264_t* p_tmem_h264)
{
    assert(NULL != p_tmem_h264);

    if (NULL != p_tmem_h264)
    {
        p_tmem_h264->p_data = NULL;
        p_tmem_h264->data_len = 0;

        if (NULL != p_tmem_h264->p_frame)
        {
            TMEM_H264_FREE(p_tmem_h264->p_frame);
        }

        TMEM_H264_FREE(p_tmem_h264);
    }
}

int tmem_h264_num(tmem_h264_t* p_tmem_h264)
{
    assert(NULL != p_tmem_h264);
    return p_tmem_h264->frame_num;
}

int tmem_h264_get(tmem_h264_t* p_tmem_h264, int idx, char** p_h264, int* p_frame_type, tmem_h264_key_t* p_key)
{
    assert(NULL != p_tmem_h264);
    assert(NULL != p_h264);
    assert(NULL != p_frame_type);

    if ((0 <= idx) && (idx < p_tmem_h264->frame_num))
    {
        *p_h264 = p_tmem_h264->p_frame[idx].p_frame;
        *p_frame_type = p_tmem_h264->p_frame[idx].type;

        if (NULL != p_key)
        {
            memcpy(p_key, &p_tmem_h264->p_frame[idx].h264_key, sizeof(tmem_h264_key_t));
        }

        return p_tmem_h264->p_frame[idx].len;
    }

    return 0; // 返回帧长度
}

int tmem_h264_get_next(tmem_h264_t* p_tmem_h264, char** p_h264, int* p_frame_type, tmem_h264_key_t* p_key)
{
    assert(NULL != p_tmem_h264);

    if (0 < p_tmem_h264->frame_num)
    {
        int idx = p_tmem_h264->idx_auto;

        p_tmem_h264->idx_auto += 1;
        if (p_tmem_h264->frame_num <= p_tmem_h264->idx_auto)
        {
            p_tmem_h264->idx_auto = 0;
        }

        return tmem_h264_get(p_tmem_h264, idx, p_h264, p_frame_type, p_key);
    }

    return 0;
}
