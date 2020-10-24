#include "ffmpeg_dec_audio.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"

#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/frame.h"
#include "libavutil/error.h"
#include "libavutil/mem.h""
#include "libswresample/swresample.h"

#include "em_buf.h"
#include <assert.h>

#define AVCODEC_MAX_AUDIO_FRAME_SIZE    192000


typedef struct ffmpeg_dec_audio_t_
{
    AVCodec*                p_codec;
    AVCodecContext*         p_context;
    struct SwrContext*      p_convert_ctx;

    AVFrame*                p_avframe;

    uint8_t audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
}ffmpeg_dec_audio_t;


ffmpeg_dec_audio_t* ffmpeg_dec_audio_create(enum AVCodecID id /*=AV_CODEC_ID_AAC*/, int tracks, int samples, int bits_per_coded_sample)
{
    ffmpeg_dec_audio_t* p_dec = KLB_MALLOC(ffmpeg_dec_audio_t, 1, 0);
    KLB_MEMSET(p_dec, 0, sizeof(ffmpeg_dec_audio_t));


    enum AVCodecID codec_id = id;// AV_CODEC_ID_AAC

    p_dec->p_codec = avcodec_find_decoder(codec_id);
    assert(NULL != p_dec->p_codec);

    p_dec->p_context = avcodec_alloc_context3(p_dec->p_codec);
    assert(NULL != p_dec->p_context);

    // 音频解码必须设置
    p_dec->p_context->channels = tracks;  // 1(单声道), 2(立体声)
    p_dec->p_context->sample_rate = samples; // 44100
    //p_dec->p_context->bit_rate = 32*1024;
    p_dec->p_context->bits_per_coded_sample = bits_per_coded_sample; // 1(8位), 2(16位), 4(32位)

    if (avcodec_open2(p_dec->p_context, p_dec->p_codec, NULL) < 0)
    {
        assert(false);
    }

    p_dec->p_convert_ctx = swr_alloc_set_opts(NULL, p_dec->p_context->channel_layout, AV_SAMPLE_FMT_S16, p_dec->p_context->sample_rate,
        p_dec->p_context->channel_layout, p_dec->p_context->sample_fmt, p_dec->p_context->sample_rate, 0, NULL);

    swr_init(p_dec->p_convert_ctx);

    return p_dec;
}

void ffmpeg_dec_audio_destroy(ffmpeg_dec_audio_t* p_dec)
{
    assert(NULL != p_dec);


    if (NULL != p_dec->p_avframe)
    {
        av_frame_free(&(p_dec->p_avframe));
    }

    if (NULL != p_dec->p_convert_ctx)
    {
        swr_free(&(p_dec->p_convert_ctx));
    }

    if (NULL != p_dec->p_context)
    {
        avcodec_close(p_dec->p_context);
        avcodec_free_context(&(p_dec->p_context));
    }

    KLB_FREE(p_dec);
}

static void ffmpeg_dec_audio_push(ffmpeg_dec_audio_t* p_dec, const char* p_data, int data_len, int64_t pts, klb_list_t* p_list)
{
    em_frame_yuv_wav_t* p_wav = KLB_MALLOC(em_frame_yuv_wav_t, 1, 0);
    memset(p_wav, 0, sizeof(em_frame_yuv_wav_t));

    p_wav->type = EM_FRAME_TYPE_WAV;
    p_wav->pts = pts;

    p_wav->sample_format = AV_SAMPLE_FMT_S16;
    p_wav->tracks = p_dec->p_context->channels;
    p_wav->samples = p_dec->p_avframe->nb_samples;

    p_wav->buf_len = data_len + 4;
    p_wav->p_buf = KLB_MALLOC(uint8_t, p_wav->buf_len, 0);
    
    memcpy(p_wav->p_buf, p_data, data_len);
    p_wav->end = data_len;

    klb_list_push_tail(p_list, p_wav);
}

int ffmpeg_dec_audio_decode(ffmpeg_dec_audio_t* p_dec, uint8_t* p_data, int data_len, int64_t pts, klb_list_t* p_list)
{
    assert(NULL != p_dec);
    assert(NULL != p_data);
    assert(NULL != p_list);

    // https://github.com/EasyDarwin/EasyAudioDecoder/blob/master/iOS/EasyAudioDecoder/FFAudioDecoder.m

    if (NULL == p_dec->p_avframe)
    {
        p_dec->p_avframe = av_frame_alloc();
    }

    int audio_num = 0;

    AVPacket packet;
    av_init_packet(&packet);

    while (0 < data_len)
    {
        packet.size = data_len;
        packet.data = p_data;

        int got_audio = 0;

        int dec_len = avcodec_decode_audio4(p_dec->p_context, p_dec->p_avframe, &got_audio, &packet);

        if (dec_len < 0)
        {
            LOG("C avcodec_decode_audio4 error!");
            assert(false);

            av_free_packet(&packet);
            return -1;
        }

        if (0 != got_audio)
        {
            uint8_t *out[] = { p_dec->audio_buf };

            int needed_buf_size = av_samples_get_buffer_size(NULL, p_dec->p_context->channels, p_dec->p_avframe->nb_samples, AV_SAMPLE_FMT_S16, 1);
            int wav_len = swr_convert(p_dec->p_convert_ctx, out, needed_buf_size, (const uint8_t **)p_dec->p_avframe->data, p_dec->p_avframe->nb_samples);
            
            if (0 < wav_len)
            {
                wav_len = wav_len * p_dec->p_context->channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

                ffmpeg_dec_audio_push(p_dec, p_dec->audio_buf, wav_len, pts, p_list);

                audio_num++;
            }
        }

        data_len -= dec_len;
        p_data += dec_len;
    }

    av_free_packet(&packet);

    return audio_num;
}
