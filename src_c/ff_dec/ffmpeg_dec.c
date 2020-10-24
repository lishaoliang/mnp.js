#include "ffmpeg_dec.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"
#include <assert.h>

#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/frame.h"
#include "libavutil/error.h"


typedef struct ffmpeg_dec_t_
{
    AVCodec*            p_codec;
    AVCodecContext*     p_context;

    AVFrame*            p_avframe;
    struct SwsContext*  p_sws_ctx;
}ffmpeg_dec_t;


void ffmpeg_dec_init()
{
    avcodec_register_all();
}

ffmpeg_dec_t* ffmpeg_dec_create(enum AVCodecID id /*=AV_CODEC_ID_H264*/)
{
    ffmpeg_dec_t* p_dec = KLB_MALLOC(ffmpeg_dec_t, 1, 0);
    KLB_MEMSET(p_dec, 0, sizeof(ffmpeg_dec_t));

    // 使用前, 需要注册ffmpeg解码器
    //avcodec_register_all();

    enum AVCodecID codec_id = id;// AV_CODEC_ID_H264, AV_CODEC_ID_H265

    p_dec->p_codec = avcodec_find_decoder(codec_id);
    assert(NULL != p_dec->p_codec);
    p_dec->p_context = avcodec_alloc_context3(p_dec->p_codec);
    assert(NULL != p_dec->p_context);

    if (p_dec->p_codec->capabilities & AV_CODEC_CAP_TRUNCATED/*CODEC_CAP_TRUNCATED*/)
    {
        // 告诉解码器传入的H26X不一定是完整的帧
        p_dec->p_context->flags |= AV_CODEC_CAP_TRUNCATED/*CODEC_FLAG_TRUNCATED*/;
    }

    if (avcodec_open2(p_dec->p_context, p_dec->p_codec, NULL) < 0)
    {
        assert(false);
    }

    return p_dec;
}

void ffmpeg_dec_destroy(ffmpeg_dec_t* p_dec)
{
    assert(NULL != p_dec);

    if (NULL != p_dec->p_avframe)
    {
        av_frame_free(&p_dec->p_avframe);
    }

    if (NULL != p_dec->p_sws_ctx)
    {
        sws_freeContext(p_dec->p_sws_ctx);
    }

    if (NULL != p_dec->p_context)
    {
        avcodec_close(p_dec->p_context);
        avcodec_free_context(&(p_dec->p_context));
    }

    KLB_FREE(p_dec);
}

AVFrame* ffmpeg_dec_decode(ffmpeg_dec_t* p_dec, uint8_t* p_data, int data_len, int64_t pts, int64_t dts)
{
    assert(NULL != p_dec);

    if (NULL == p_dec->p_avframe)
    {
        p_dec->p_avframe = av_frame_alloc();
    }

    // 解码数据
    int got_picture = 0;

    AVPacket packet = { 0 };
    packet.data = p_data;
    packet.size = data_len;
    packet.pts = pts;
    packet.dts = dts;

    avcodec_decode_video2(p_dec->p_context, p_dec->p_avframe, &got_picture, &packet);

    if (0 != got_picture)
    {
        if (NULL != p_dec->p_avframe)
        {
            AVFrame* p_frame = p_dec->p_avframe;
            p_dec->p_avframe = NULL;

            return p_frame;
        }
    }

    return NULL; //未得到帧, 还需要继续送数据
}

void ffmpeg_dec_av_frame_free(AVFrame* p_frame)
{
    if (NULL != p_frame)
    {
        av_frame_free(&p_frame);
    }
}

static em_frame_yuv_wav_t* ffmpeg_dec_yuv420p(AVFrame* p_avframe, int64_t pts)
{
    int w = p_avframe->width;
    int h = p_avframe->height;

    int uv_w = w / 2;
    int uv_h = h / 2;

    // Opengl纹理, 需要4的倍数
    int pitch_y = (w >> 2) << 2;    //(w + 3) / 4 * 4;
    int pitch_u = (uv_w >> 2) << 2; //(uv_w + 3) / 4 * 4;
    int pitch_v = (uv_w >> 2) << 2; //(uv_w + 3) / 4 * 4;

    // 防止越界
    pitch_y = MIN(pitch_y, p_avframe->linesize[0]);
    pitch_u = MIN(pitch_u, p_avframe->linesize[1]);
    pitch_v = MIN(pitch_v, p_avframe->linesize[2]);

    int y_len = pitch_y * h;
    int u_len = pitch_u * uv_h;
    int v_len = pitch_v * uv_h;

    em_frame_yuv_wav_t* p_yuv = KLB_MALLOC(em_frame_yuv_wav_t, 1, 0);
    KLB_MEMSET(p_yuv, 0, sizeof(em_frame_yuv_wav_t));

    p_yuv->type = EM_FRAME_TYPE_YUV;
    p_yuv->w = w;
    p_yuv->h = h;

    p_yuv->buf_len = y_len + u_len + v_len;
    p_yuv->end = y_len + u_len + v_len;
    p_yuv->p_buf = KLB_MALLOC(uint8_t, p_yuv->buf_len, 0);

    p_yuv->p_y = p_yuv->p_buf;
    p_yuv->p_u = p_yuv->p_buf + y_len;
    p_yuv->p_v = p_yuv->p_buf + y_len + u_len;

    p_yuv->pitch_y = pitch_y;
    p_yuv->h_y = h;
    p_yuv->pitch_u = pitch_u;
    p_yuv->h_u = uv_h;
    p_yuv->pitch_v = pitch_v;
    p_yuv->h_v = uv_h;

    // Y
    uint8_t* p_src = p_avframe->data[0];
    uint8_t* p_dst = (uint8_t*)(p_yuv->p_y);
    for (int i = 0; i < h; i++)
    {
        memcpy(p_dst, p_src, pitch_y/*p_avframe->width*/);
        p_src += p_avframe->linesize[0];
        p_dst += pitch_y;
    }

    //U
    p_src = p_avframe->data[1];
    p_dst = (uint8_t*)(p_yuv->p_u);
    for (int i = 0; i < uv_h; i++)
    {
        memcpy(p_dst, p_src, pitch_u/*uv_w*/);
        p_src += p_avframe->linesize[1];
        p_dst += pitch_u;
    }

    //V
    p_src = p_avframe->data[2];
    p_dst = (uint8_t*)(p_yuv->p_v);
    for (int i = 0; i < uv_h; i++)
    {
        memcpy(p_dst, p_src, pitch_v/*uv_w*/);
        p_src += p_avframe->linesize[2];
        p_dst += pitch_v;
    }

    p_yuv->pts = pts;//p_avframe->pts;

    return p_yuv;
}

em_frame_yuv_wav_t* ffmpeg_dec_decode2(ffmpeg_dec_t* p_dec, uint8_t* p_data, int data_len, int64_t pts, int64_t dts)
{
    AVFrame* p_frame = ffmpeg_dec_decode(p_dec, p_data, data_len, pts, dts);

    if (NULL != p_frame)
    {
        em_frame_yuv_wav_t* p_yuv = ffmpeg_dec_yuv420p(p_frame, pts);
        ffmpeg_dec_av_frame_free(p_frame);

        return p_yuv;
    }

    return NULL;
}

void ffmpeg_dec_yuv_frame_free(em_frame_yuv_wav_t* p_yuv)
{
    assert(NULL != p_yuv);

    KLB_FREE(p_yuv->p_buf);
    KLB_FREE(p_yuv);
}

