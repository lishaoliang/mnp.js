#include "em_frame_yuv_wav.h"
#include "mem/klb_mem.h"
#include <assert.h>

em_frame_yuv_wav_t* em_frame_yuv_wav_malloc(int type, int buf_len)
{
    assert(EM_FRAME_TYPE_YUV == type || EM_FRAME_TYPE_WAV == type);

    em_frame_yuv_wav_t* p_frame = KLB_MALLOC(em_frame_yuv_wav_t, 1, 0);
    KLB_MEMSET(p_frame, 0, sizeof(em_frame_yuv_wav_t));

    p_frame->type = type;
    p_frame->buf_len = buf_len;

    p_frame->p_buf = KLB_MALLOC(uint8_t, p_frame->buf_len, 0);

    return p_frame;
}

void em_frame_yuv_wav_free(em_frame_yuv_wav_t* p_frame)
{
    assert(NULL != p_frame);

    KLB_FREE(p_frame->p_buf);
    KLB_FREE(p_frame);
}
