#include "em_audio_ctx.h"
#include <SDL/SDL.h>
//#include <SDL/SDL_mixer.h>
#include <SDL/SDL_audio.h>
#include "mem/klb_mem.h"
#include "list/klb_list.h"
#include "em_log.h"
#include <assert.h>

/*
Mix_OpenAudio(0, 0, 0, 0);
Mix_PlayChannel(0, NULL, 1);
SDL_RWFromMem(NULL, 0);
Mix_LoadWAV_RW(NULL, 0);
*/


typedef struct em_audio_ctx_t_
{
    SDL_AudioSpec   audio_spec;

    klb_list_t*     p_wav_list;
}em_audio_ctx_t;

em_audio_ctx_t* g_em_audio_ctx = NULL;

static void callback_em_audio_ctx(void* userdata, Uint8* stream, int len);


int em_audio_ctx_init()
{
    g_em_audio_ctx = KLB_MALLOC(em_audio_ctx_t, 1, 0);
    KLB_MEMSET(g_em_audio_ctx, 0, sizeof(em_audio_ctx_t));

    g_em_audio_ctx->p_wav_list = klb_list_create();

    g_em_audio_ctx->audio_spec.freq = 44100;            /**< DSP frequency -- samples per second */
    g_em_audio_ctx->audio_spec.format = AUDIO_S16LSB;   /**< Audio data format */
    g_em_audio_ctx->audio_spec.channels = 2;            /**< Number of channels: 1 mono, 2 stereo */
    g_em_audio_ctx->audio_spec.silence = 0;             /**< Audio buffer silence value (calculated) */
    
    g_em_audio_ctx->audio_spec.samples = 4096;          /**< Audio buffer size in samples (power of 2) */
    //g_em_audio_ctx->audio_spec.padding = 0;           /**< Necessary for some compile environments */
    //g_em_audio_ctx->audio_spec.size = 4096;           /**< Audio buffer size in bytes (calculated) */
    g_em_audio_ctx->audio_spec.callback = callback_em_audio_ctx;
    g_em_audio_ctx->audio_spec.userdata = g_em_audio_ctx;

    if (0 != SDL_OpenAudio(&g_em_audio_ctx->audio_spec, NULL))
    {
        LOG("C SDL_OpenAudio error!");
    }

    SDL_PauseAudio(0);

    LOG("C em_audio_ctx_init ok.");
}

void em_audio_ctx_quit()
{
    if (NULL != g_em_audio_ctx)
    {
        SDL_CloseAudio();

        while (0 < klb_list_size(g_em_audio_ctx->p_wav_list))
        {
            em_frame_yuv_wav_t* p_tmp = (em_frame_yuv_wav_t*)klb_list_pop_head(g_em_audio_ctx->p_wav_list);

            KLB_FREE_BY(p_tmp, em_frame_yuv_wav_free);
        }

        KLB_FREE_BY(g_em_audio_ctx->p_wav_list, klb_list_destroy);
        KLB_FREE(g_em_audio_ctx);
    }
}

void em_audio_ctx_play()
{
    SDL_PauseAudio(0);
}

void em_audio_ctx_pause()
{
    SDL_PauseAudio(1);
}

int em_audio_ctx_push(em_frame_yuv_wav_t* p_wav)
{
    assert(NULL != p_wav);
    assert(EM_FRAME_TYPE_WAV == p_wav->type);

    klb_list_push_tail(g_em_audio_ctx->p_wav_list, p_wav);

    return 0;
}

int em_audio_ctx_push2(klb_list_t* p_list)
{
    while (0 < klb_list_size(p_list))
    {
        em_frame_yuv_wav_t* p_wav = (em_frame_yuv_wav_t*)klb_list_pop_head(p_list);
        em_audio_ctx_push(p_wav);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////

static void callback_em_audio_ctx(void* p_userdata, Uint8* p_stream, int stream_len)
{
    em_audio_ctx_t* p_audio_ctx = (em_audio_ctx_t*)p_userdata;
    //LOG("callback_em_audio_ctx:[%p],[%p,%d]", userdata, stream, len);

    uint8_t* p_dst = (uint8_t*)p_stream;
    int dst_len = stream_len;

    memset(p_dst, 0, dst_len);

    while (0 < dst_len && 0 < klb_list_size(p_audio_ctx->p_wav_list))
    {
        em_frame_yuv_wav_t* p_wav = (em_frame_yuv_wav_t*)klb_list_head(p_audio_ctx->p_wav_list);
        if (NULL != p_wav)
        {
            int cp_len = MIN(dst_len, p_wav->end - p_wav->start);

            if (0 < cp_len)
            {
                memcpy(p_dst, p_wav->p_buf + p_wav->start, cp_len);

                p_wav->start += cp_len;
                dst_len -= cp_len;
                p_dst += cp_len;
            }

            if (p_wav->end <= p_wav->start)
            {
                em_frame_yuv_wav_t* p_tmp = (em_frame_yuv_wav_t*)klb_list_pop_head(p_audio_ctx->p_wav_list);

                KLB_FREE_BY(p_tmp, em_frame_yuv_wav_free);
            }
        }
        else
        {
            break;
        }
    }
}
