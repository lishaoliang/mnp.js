#ifndef __EM_WS_FLV_H__
#define __EM_WS_FLV_H__

#include "klb_type.h"
#include "em_ws/em_ws.h"
#include "em_net/em_conn_manage.h"

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct emws_flv_t_ emws_flv_t;


void emws_flv_init_env(em_conn_env_t* p_env, emws_flv_t* p_wsflv);

emws_flv_t* emws_flv_create(const char* p_name, em_conn_manage_t* p_conn_manage, emws_socket_t* p_socket);
void emws_flv_destroy(void* ptr);


int emws_flv_send(void* ptr, em_buf_t* p_buf);
int emws_flv_send_md(void* ptr);

int emws_flv_on_open(emws_socket_t* p_emws, uint32_t now_ticks, int code);
int emws_flv_on_error(emws_socket_t* p_emws, uint32_t now_ticks);
int emws_flv_on_close(emws_socket_t* p_emws, uint32_t now_ticks);
int emws_flv_on_recv(emws_socket_t* p_emws, uint32_t now_ticks, const EmscriptenWebSocketMessageEvent* p_message);
int emws_flv_on_send(emws_socket_t* p_socket, uint32_t now_ticks);


#ifdef __cplusplus
}
#endif

#endif // __EM_WS_FLV_H__
//end
