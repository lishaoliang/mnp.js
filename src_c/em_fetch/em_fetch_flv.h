#ifndef __EM_FETCH_FLV_H__
#define __EM_FETCH_FLV_H__


#include "klb_type.h"
#include "em_fetch/em_fetch_stream.h"
#include "em_net/em_conn_manage.h"


#if defined(__cplusplus)
extern "C" {
#endif

typedef struct emfetch_flv_t_ emfetch_flv_t;

void emfetch_flv_init_env(em_conn_env_t* p_env, emfetch_flv_t* p_flv);

emfetch_flv_t* emfetch_flv_create(const char* p_name, em_conn_manage_t* p_conn_manage, emfetch_stream_t* p_emfetch);
void emfetch_flv_destroy(void* ptr);


int emfetch_flv_on_open(emfetch_stream_t* p_emfetch, uint32_t now_ticks, int code);
int emfetch_flv_on_error(emfetch_stream_t* p_emfetch, uint32_t now_ticks);
int emfetch_flv_on_recv(emfetch_stream_t* p_emfetch, uint32_t now_ticks, emscripten_fetch_t* p_fetch);


#ifdef __cplusplus
}
#endif

#endif // __EM_FETCH_FLV_H__
//end
