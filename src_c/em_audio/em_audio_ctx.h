#ifndef __EM_AUDIO_CTX_H__
#define __EM_AUDIO_CTX_H__

#include "klb_type.h"
#include "em_frame_yuv_wav.h"
#include "list/klb_list.h"

#if defined(__cplusplus)
extern "C" {
#endif


int em_audio_ctx_init();
void em_audio_ctx_quit();

void em_audio_ctx_play();
void em_audio_ctx_pause();

int em_audio_ctx_push(em_frame_yuv_wav_t* p_wav);
int em_audio_ctx_push2(klb_list_t* p_list);


#ifdef __cplusplus
}
#endif

#endif // __EM_AUDIO_CTX_H__
//end
