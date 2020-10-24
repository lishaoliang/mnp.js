#include "ff_dec_single.h"
#include "mem/klb_mem.h"
#include "em_util/em_mnp_avcode.h"
#include "em_util/em_log.h"
#include "mnp/klb_mnp.h"
#include <assert.h>


static int ff_dec_manage_proc(ff_dec_single_t* p_ff_dec, bool hidden);

#ifdef __EMSCRIPTEN_PTHREADS__
static int ff_dec_manage_thread(void* p_obj, int* p_run)
{
    ff_dec_single_t* p_ff_dec = (ff_dec_single_t*)p_obj;
    assert(NULL != p_ff_dec);

    while (0 != *p_run)
    {
        //LOG("ff_dec_manage_thread");
        ff_dec_manage_proc(p_ff_dec);

        klb_sleep(10);
    }

    return 0;
}
#endif


ff_dec_single_t* ff_dec_single_create()
{
    ff_dec_single_t* p_ff_dec = KLB_MALLOC(ff_dec_single_t, 1, 0);
    KLB_MEMSET(p_ff_dec, 0, sizeof(ff_dec_single_t));

    p_ff_dec->p_video_list = klb_list_create();
    p_ff_dec->p_yuv_list = klb_list_create();

    p_ff_dec->need_key = true;

    // 速率控制
    for (int i = 0; i < FF_RATE_CTRL_MAX; i++)
    {
        p_ff_dec->p_rate_ctrl[i] = ff_rate_ctrl_create(p_ff_dec, i);
    }
    p_ff_dec->p_rate_ctrl_active = p_ff_dec->p_rate_ctrl[FF_RATE_CTRL_LOW];


    // 音频
    p_ff_dec->p_audio_list = klb_list_create();
    p_ff_dec->p_wav_list = klb_list_create();


    return p_ff_dec;
}

void ff_dec_single_destroy(ff_dec_single_t* p_ff_dec)
{
    assert(NULL != p_ff_dec);

    while (0 < klb_list_size(p_ff_dec->p_yuv_list))
    {
        em_frame_yuv_wav_t* p_yuv = (em_frame_yuv_wav_t*)klb_list_pop_head(p_ff_dec->p_yuv_list);
        KLB_FREE_BY(p_yuv, em_frame_yuv_wav_free);
    }

    while (0 < klb_list_size(p_ff_dec->p_video_list))
    {
        em_buf_t* p_buf = (em_buf_t*)klb_list_pop_head(p_ff_dec->p_video_list);
        KLB_FREE_BY(p_buf, em_buf_unref_next);
    }

    while (0 < klb_list_size(p_ff_dec->p_wav_list))
    {
        em_frame_yuv_wav_t* p_wav = (em_frame_yuv_wav_t*)klb_list_pop_head(p_ff_dec->p_yuv_list);
        KLB_FREE_BY(p_wav, em_frame_yuv_wav_free);
    }

    while (0 < klb_list_size(p_ff_dec->p_audio_list))
    {
        em_buf_t* p_buf = (em_buf_t*)klb_list_pop_head(p_ff_dec->p_video_list);
        KLB_FREE_BY(p_buf, em_buf_unref_next);
    }

    for (int i = 0; i < FF_RATE_CTRL_MAX; i++)
    {
        KLB_FREE_BY(p_ff_dec->p_rate_ctrl[i], ff_rate_ctrl_destroy);
    }

    KLB_FREE_BY(p_ff_dec->p_wav_list, klb_list_destroy);
    KLB_FREE_BY(p_ff_dec->p_audio_list, klb_list_destroy);
    KLB_FREE_BY(p_ff_dec->p_yuv_list, klb_list_destroy);
    KLB_FREE_BY(p_ff_dec->p_video_list, klb_list_destroy);

    KLB_FREE_BY(p_ff_dec->p_dec, ffmpeg_dec_destroy);
    KLB_FREE(p_ff_dec);
}

int ff_dec_single_start(ff_dec_single_t* p_ff_dec)
{
    assert(NULL != p_ff_dec);

#ifdef __EMSCRIPTEN_PTHREADS__
    assert(NULL == p_ff_dec->p_thread);

    p_ff_dec->p_thread = klb_thread_create(ff_dec_manage_thread, p_ff_dec, -1, "ff_dec_manage_thread");

    return (NULL != p_ff_dec->p_thread) ? 0 : 1;
#else
    return 0;
#endif

}

void ff_dec_single_stop(ff_dec_single_t* p_ff_dec)
{
    assert(NULL != p_ff_dec);

#ifdef __EMSCRIPTEN_PTHREADS__
    KLB_FREE_BY(p_ff_dec->p_thread, klb_thread_destroy);
#endif

}

int ff_dec_single_run(ff_dec_single_t* p_ff_dec, int64_t now_ticks, bool hidden)
{
#if !defined(__EMSCRIPTEN_PTHREADS__)
    return ff_dec_manage_proc(p_ff_dec, hidden);
#endif

    return 0;
}

int ff_dec_single_push(ff_dec_single_t* p_ff_dec, em_buf_t* p_buf)
{
    assert(NULL != p_ff_dec);

    klb_mnp_media_t* p_media = (klb_mnp_media_t*)p_buf->p_buf;
    if (KLB_MNP_DTYPE_H264 == p_media->dtype ||
        KLB_MNP_DTYPE_H265 == p_media->dtype)
    {
        klb_list_push_tail(p_ff_dec->p_video_list, p_buf);
    }
    else if (KLB_MNP_DTYPE_AAC == p_media->dtype)
    {
        klb_list_push_tail(p_ff_dec->p_audio_list, p_buf);
    }
    else
    {
        KLB_FREE_BY(p_buf, em_buf_unref_next);
    }

    return 0;
}

em_frame_yuv_wav_t* ff_dec_single_get(ff_dec_single_t* p_ff_dec, int64_t now_ticks)
{
    assert(NULL != p_ff_dec);

    return p_ff_dec->p_rate_ctrl_active->get_frame(p_ff_dec->p_rate_ctrl_active, now_ticks);
}

void ff_dec_single_free(em_frame_yuv_wav_t* p_yuv)
{
    assert(NULL != p_yuv);

    ffmpeg_dec_yuv_frame_free(p_yuv);
}

void ff_dec_single_set_delay(ff_dec_single_t* p_ff_dec, ff_rate_ctrl_type_e delay)
{
    p_ff_dec->p_rate_ctrl_active = p_ff_dec->p_rate_ctrl[delay];
    p_ff_dec->p_rate_ctrl_active->reset(p_ff_dec->p_rate_ctrl_active);
}

//////////////////////////////////////////////////////////////////////////

static ffmpeg_dec_t* ff_dec_manage_get_dec(ff_dec_single_t* p_ff_dec, enum AVCodecID code_id)
{
    if (AV_CODEC_ID_NONE == code_id)
    {
        return NULL;
    }

    if (p_ff_dec->dec_code_id != code_id)
    {
        KLB_FREE_BY(p_ff_dec->p_dec, ffmpeg_dec_destroy);
        p_ff_dec->dec_code_id = AV_CODEC_ID_NONE;
    }

    if (AV_CODEC_ID_H264 != code_id && AV_CODEC_ID_H265 != code_id)
    {
        return NULL; // 不支持的格式
    }

    if (NULL == p_ff_dec->p_dec)
    {
        p_ff_dec->p_dec = ffmpeg_dec_create(code_id);
        p_ff_dec->dec_code_id = code_id;
    }

    return p_ff_dec->p_dec;
}

static ffmpeg_dec_audio_t* ff_dec_manage_get_dec_audio(ff_dec_single_t* p_ff_dec)
{
    if (NULL == p_ff_dec->p_dec_audio)
    {
        p_ff_dec->p_dec_audio = ffmpeg_dec_audio_create(AV_CODEC_ID_AAC, 2, 44100, 2);
    }

    return p_ff_dec->p_dec_audio;
}

static em_buf_t* ff_dec_manage_get_next(ff_dec_single_t* p_ff_dec)
{
    em_buf_t* p_video = (em_buf_t*)klb_list_head(p_ff_dec->p_video_list);
    em_buf_t* p_audio = (em_buf_t*)klb_list_head(p_ff_dec->p_audio_list);

    if (NULL == p_video && NULL == p_audio)
    {
        return NULL; // 无数据
    }
    
    if (NULL == p_video || NULL == p_audio)
    {
        // 一方有数据
        if (NULL != p_video)
        {
            return (em_buf_t*)klb_list_pop_head(p_ff_dec->p_video_list);
        }

        if (NULL != p_audio)
        {
            return (em_buf_t*)klb_list_pop_head(p_ff_dec->p_audio_list);
        }

        assert(false);
        return NULL;
    }

    // 都有数据
    klb_mnp_media_t* p_video_media = p_video->p_buf;
    klb_mnp_media_t* p_audio_media = p_audio->p_buf;

    if (p_audio_media->time <= p_video_media->time)
    {
        return (em_buf_t*)klb_list_pop_head(p_ff_dec->p_audio_list);
    }
    else
    {
        return (em_buf_t*)klb_list_pop_head(p_ff_dec->p_video_list);
    }
}

static void ff_dec_manage_drop_data(ff_dec_single_t* p_ff_dec, bool hidden)
{
    int drop_num = p_ff_dec->p_rate_ctrl_active->drop_data(p_ff_dec->p_rate_ctrl_active, hidden);

    if (0 < drop_num)
    {
        // 丢数据之后, 已经解码的YUV帧序列时间戳, 失去意义了
        klb_list_iter_t* p_iter = klb_list_begin(p_ff_dec->p_yuv_list);
        while (NULL != p_iter)
        {
            em_frame_yuv_wav_t* p_yuv = (em_frame_yuv_wav_t*)klb_list_data(p_iter);
            p_yuv->pts = 0;

            p_iter = klb_list_next(p_iter);
        }

        // 重置
        p_ff_dec->p_rate_ctrl_active->reset(p_ff_dec->p_rate_ctrl_active);

        // 需要解码关键帧
        p_ff_dec->need_key = true;
    }
}

static int ff_dec_manage_proc(ff_dec_single_t* p_ff_dec, bool hidden)
{
    assert(NULL != p_ff_dec);

    // 是否丢数据
    ff_dec_manage_drop_data(p_ff_dec, hidden);


    // 解码
    int yuv_max = 4;

    while (klb_list_size(p_ff_dec->p_yuv_list) < yuv_max)
    {
        em_buf_t* p_buf = ff_dec_manage_get_next(p_ff_dec);

        if (NULL != p_buf)
        {
            klb_mnp_media_t* p_media = (klb_mnp_media_t*)p_buf->p_buf;
            uint8_t* p_data = p_buf->p_buf + sizeof(klb_mnp_media_t);
            int data_len = p_buf->end - sizeof(klb_mnp_media_t);

            if (KLB_MNP_DTYPE_H264 == p_media->dtype ||
                KLB_MNP_DTYPE_H265 == p_media->dtype)
            {
                if (KLB_MNP_VTYPE_I == p_media->vtype)
                {
                    p_ff_dec->need_key = false;
                }

                if (!p_ff_dec->need_key)
                {
                    // 解码
                    ffmpeg_dec_t* p_dec = ff_dec_manage_get_dec(p_ff_dec, em_mnp_to_avcodec((klb_mnp_dtype_e)p_media->dtype));
                    if (NULL != p_dec)
                    {
                        //LOG("C ffmpeg_dec_decode2 time:[%lld]", p_media->time / 1000);
                        em_frame_yuv_wav_t* p_yuv = ffmpeg_dec_decode2(p_ff_dec->p_dec, p_data, data_len, p_media->time, p_media->time);
                        if (NULL != p_yuv)
                        {
                            klb_list_push_tail(p_ff_dec->p_yuv_list, p_yuv);
                        }
                    }
                    else
                    {
                        //LOG("C ff_dec_manage_get_dec error!");
                    }
                }
            }
            else if (KLB_MNP_DTYPE_AAC == p_media->dtype)
            {
                if (!p_ff_dec->need_key)
                {
                    ffmpeg_dec_audio_t* p_dec_audio = ff_dec_manage_get_dec_audio(p_ff_dec);
                    if (NULL != p_dec_audio)
                    {
                        ffmpeg_dec_audio_decode(p_dec_audio, p_data, data_len, p_media->time, p_ff_dec->p_wav_list);
                    }
                }
            }

            KLB_FREE_BY(p_buf, em_buf_unref_next);
        }
        else
        {
            break;
        }
    }

    return 0;
}
