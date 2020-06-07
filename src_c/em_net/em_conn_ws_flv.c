#include "em_conn_ws_flv.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"
#include <assert.h>


typedef struct em_conn_ws_flv_t_
{
    em_conn_manage_t*   p_conn_manage;
    em_socket_t*        p_socket;
}em_conn_ws_flv_t;


