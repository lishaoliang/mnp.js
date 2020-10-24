#ifndef __EM_FETCH_MNP_H__
#define __EM_FETCH_MNP_H__

#include "klb_type.h"
#include "em_fetch/em_fetch_stream.h"
#include "em_net/em_conn_manage.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct emfetch_mnp_t_ emfetch_mnp_t;

void emfetch_mnp_init_env(em_conn_env_t* p_env, emfetch_mnp_t* p_flv);

emfetch_mnp_t* emfetch_mnp_create(const char* p_name, em_conn_manage_t* p_conn_manage, emfetch_stream_t* p_emfetch);
void emfetch_mnp_destroy(void* ptr);


int emfetch_mnp_on_open(emfetch_stream_t* p_emfetch, uint32_t now_ticks, int code);
int emfetch_mnp_on_error(emfetch_stream_t* p_emfetch, uint32_t now_ticks);
int emfetch_mnp_on_recv(emfetch_stream_t* p_emfetch, uint32_t now_ticks, const char* p_data, int data_len);


#ifdef __cplusplus
}
#endif

#endif // __EM_FETCH_MNP_H__
//end
