#include "ff_dec/ff_dec_manage.h"
#include "mem/klb_mem.h"
#include "ff_dec/ffmpeg_dec.h"
#include "em_util/em_mnp_avcode.h"
#include "thread/klb_thread.h"
#include "list/klb_list.h"
#include "em_util/em_log.h"
#include "mnp/klb_mnp.h"
#include <assert.h>

typedef struct ff_dec_manage_t_
{
    klb_list_t*     p_frame_list;   // 待解码的帧列表
    klb_list_t*     p_yuv_list;     // 已经解码完毕的yuv数据列表

    ffmpeg_dec_t*   p_dec;          // 解码器
    enum AVCodecID  dec_code_id;    // 解码器ID

#ifdef __EMSCRIPTEN_PTHREADS__
    klb_thread_t*   p_thread;
#endif
}ff_dec_manage_t;


static int ff_dec_manage_proc(ff_dec_manage_t* p_manage);

#ifdef __EMSCRIPTEN_PTHREADS__
static int ff_dec_manage_thread(void* p_obj, int* p_run)
{
    ff_dec_manage_t* p_manage = (ff_dec_manage_t*)p_obj;
    assert(NULL != p_manage);

    while (0 != *p_run)
    {
        //LOG("ff_dec_manage_thread");
        ff_dec_manage_proc(p_manage);

        klb_sleep(10);
    }

    return 0;
}
#endif


ff_dec_manage_t* ff_dec_manage_create()
{
    ff_dec_manage_t* p_manage = KLB_MALLOC(ff_dec_manage_t, 1, 0);
    KLB_MEMSET(p_manage, 0, sizeof(ff_dec_manage_t));

    p_manage->p_frame_list = klb_list_create();
    p_manage->p_yuv_list = klb_list_create();

    return p_manage;
}

void ff_dec_manage_destroy(ff_dec_manage_t* p_manage)
{
    assert(NULL != p_manage);

    while (0 < klb_list_size(p_manage->p_yuv_list))
    {
        em_yuv_frame_t* p_yuv = (em_yuv_frame_t*)klb_list_pop_head(p_manage->p_yuv_list);
        ffmpeg_dec_yuv_frame_free(p_yuv);
    }

    while (0 < klb_list_size(p_manage->p_frame_list))
    {
        em_buf_t* p_buf = (em_buf_t*)klb_list_pop_head(p_manage->p_frame_list);

        KLB_FREE_BY(p_buf, em_buf_unref_next);
    }

    KLB_FREE_BY(p_manage->p_yuv_list, klb_list_destroy);
    KLB_FREE_BY(p_manage->p_frame_list, klb_list_destroy);

    KLB_FREE_BY(p_manage->p_dec, ffmpeg_dec_destroy);
    KLB_FREE(p_manage);
}

int ff_dec_manage_start(ff_dec_manage_t* p_manage)
{
    assert(NULL != p_manage);

#ifdef __EMSCRIPTEN_PTHREADS__
    assert(NULL == p_manage->p_thread);

    p_manage->p_thread = klb_thread_create(ff_dec_manage_thread, p_manage, -1, "ff_dec_manage_thread");

    return (NULL != p_manage->p_thread) ? 0 : 1;
#else
    return 0;
#endif

}

void ff_dec_manage_stop(ff_dec_manage_t* p_manage)
{
    assert(NULL != p_manage);

#ifdef __EMSCRIPTEN_PTHREADS__
    KLB_FREE_BY(p_manage->p_thread, klb_thread_destroy);
#endif

}

int ff_dec_manage_run(ff_dec_manage_t* p_manage, uint32_t now_ticks)
{
#if !defined(__EMSCRIPTEN_PTHREADS__)
    return ff_dec_manage_proc(p_manage);
#endif

    return 0;
}

int ff_dec_manage_push(ff_dec_manage_t* p_manage, em_buf_t* p_buf)
{
    assert(NULL != p_manage);

    klb_list_push_tail(p_manage->p_frame_list, p_buf);

    return 0;
}

em_yuv_frame_t* ff_dec_manage_get(ff_dec_manage_t* p_manage)
{
    assert(NULL != p_manage);

    em_yuv_frame_t* p_yuv = (em_yuv_frame_t*)klb_list_pop_head(p_manage->p_yuv_list);

    return p_yuv;
}

void ff_dec_manage_free(em_yuv_frame_t* p_yuv)
{
    assert(NULL != p_yuv);

    ffmpeg_dec_yuv_frame_free(p_yuv);
}

//////////////////////////////////////////////////////////////////////////

static ffmpeg_dec_t* ff_dec_manage_get_dec(ff_dec_manage_t* p_manage, enum AVCodecID code_id)
{
    if (AV_CODEC_ID_NONE == code_id)
    {
        return NULL;
    }

    if (p_manage->dec_code_id != code_id)
    {
        KLB_FREE_BY(p_manage->p_dec, ffmpeg_dec_destroy);
        p_manage->dec_code_id = AV_CODEC_ID_NONE;
    }

    assert(AV_CODEC_ID_H264 == code_id);

    if (NULL == p_manage->p_dec)
    {
        p_manage->p_dec = ffmpeg_dec_create(code_id);
        p_manage->dec_code_id = code_id;
    }

    return p_manage->p_dec;
}

static em_buf_t* ff_dec_manage_get_frame(ff_dec_manage_t* p_manage)
{
    em_buf_t* p_buf = (em_buf_t*)klb_list_pop_head(p_manage->p_frame_list);

    return p_buf;
}

static int ff_dec_manage_proc(ff_dec_manage_t* p_manage)
{
    assert(NULL != p_manage);

    em_buf_t* p_buf = ff_dec_manage_get_frame(p_manage);

    if (NULL != p_buf)
    {
        klb_mnp_md_t* p_media = (klb_mnp_md_t*)p_buf->p_buf;
        uint8_t* p_data = p_buf->p_buf + sizeof(klb_mnp_md_t);
        int data_len = p_buf->end - sizeof(klb_mnp_md_t);

        // 解码
        ffmpeg_dec_t* p_dec = ff_dec_manage_get_dec(p_manage, em_mnp_to_avcodec((klb_mnp_dtype_e)p_media->dtype));
        if (NULL != p_dec)
        {
            em_yuv_frame_t* p_yuv = ffmpeg_dec_decode2(p_manage->p_dec, p_data, data_len, p_media->time, p_media->time);
            if (NULL != p_yuv)
            {
                klb_list_push_tail(p_manage->p_yuv_list, p_yuv);
            }
        }

        KLB_FREE_BY(p_buf, em_buf_unref_next);
    }

    return 0;
}
