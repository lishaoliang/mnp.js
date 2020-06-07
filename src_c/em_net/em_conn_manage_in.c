#include "em_net/em_conn_manage_in.h"
#include "mem/klb_mem.h"
#include "em_util/em_buf_mnp.h"
#include <assert.h>

int em_conn_manage_push(em_conn_manage_t* p_manage, const char* p_protocol, const char* p_name, int code, em_buf_t* p_buf)
{
    assert(NULL != p_manage);

    if (NULL != p_buf)
    {
        assert(0 == em_buf_mnp_check(p_buf));
    }

    em_conn_manage_packet_txt_t* p_pack_txt = KLB_MALLOC(em_conn_manage_packet_txt_t, 1, 0);
    KLB_MEMSET(p_pack_txt, 0, sizeof(em_conn_manage_packet_txt_t));

    p_pack_txt->code = code;
    p_pack_txt->p_buf = p_buf;

    strncpy(p_pack_txt->protocol, p_protocol, EM_NET_NAME_LEN);
    strncpy(p_pack_txt->name, p_name, EM_NET_NAME_LEN);

    klb_list_push_tail(p_manage->p_read_txt_list, p_pack_txt);

    return 0;
}

int em_conn_manage_push_md(em_conn_manage_t* p_manage, const char* p_protocol, const char* p_name, em_buf_t* p_buf)
{
    assert(NULL != p_manage);
    assert(NULL != p_buf);
    //assert(0 == em_buf_mnp_check(p_buf));

    em_conn_manage_packet_md_t* p_pack_md = KLB_MALLOC(em_conn_manage_packet_md_t, 1, 0);
    KLB_MEMSET(p_pack_md, 0, sizeof(em_conn_manage_packet_md_t));

    p_pack_md->p_buf = p_buf;

    strncpy(p_pack_md->protocol, p_protocol, EM_NET_NAME_LEN);
    strncpy(p_pack_md->name, p_name, EM_NET_NAME_LEN);

    klb_list_push_tail(p_manage->p_read_md_list, p_pack_md);

    return 0;
}
