#include "em_conn_manage.h"
#include "em_conn_manage_in.h"
#include "em_socket.h"
#include "mem/klb_mem.h"
#include "hash/klb_hlist.h"
#include "em_util/em_buf.h"
#include "em_util/em_log.h"
#include "em_util/em_buf_mnp.h"
#include "em_fetch_flv.h"
#include "em_fetch_mnp.h"
#include "em_ws_flv.h"
#include "em_ws_mnp.h"
#include <assert.h>


em_conn_manage_t* em_conn_manage_create(em_socket_manage_t* p_socket_manage)
{
    assert(NULL != p_socket_manage);

    em_conn_manage_t* p_manage = KLB_MALLOC(em_conn_manage_t, 1, 0);
    KLB_MEMSET(p_manage, 0, sizeof(em_conn_manage_t));

    p_manage->p_socket_manage = p_socket_manage;

    p_manage->p_conn_hlist = klb_hlist_create(EM_SOCKET_HASH_TABLE_MAX);

    p_manage->p_read_txt_list = klb_list_create();
    p_manage->p_read_md_list = klb_list_create();

    return p_manage;
}

void em_conn_manage_destroy(em_conn_manage_t* p_manage)
{
    assert(NULL != p_manage);

    while (0 < klb_hlist_size(p_manage->p_conn_hlist))
    {
        klb_hlist_iter_t* p_iter = klb_hlist_begin(p_manage->p_conn_hlist);

        uint32_t key_len = 0;
        void* p_key = klb_hlist_key(p_iter, &key_len);
        em_socket_manage_remove(p_manage->p_socket_manage, p_key, key_len);

        em_conn_env_t* p_env = (em_conn_env_t*)klb_hlist_pop_head(p_manage->p_conn_hlist);
        p_env->cb_destroy(p_env->p_conn);
        KLB_FREE(p_env);
    }

    while (0 < klb_list_size(p_manage->p_read_txt_list))
    {
        em_conn_manage_packet_txt_t* p_pack_txt = (em_conn_manage_packet_txt_t*)klb_list_pop_head(p_manage->p_read_txt_list);
        em_buf_unref_next(p_pack_txt->p_buf);

        KLB_FREE(p_pack_txt);
    }

    while (0 < klb_list_size(p_manage->p_read_md_list))
    {
        em_conn_manage_packet_md_t* p_pack_md = (em_conn_manage_packet_md_t*)klb_list_pop_head(p_manage->p_read_md_list);
        em_buf_unref_next(p_pack_md->p_buf);

        KLB_FREE(p_pack_md);
    }

    KLB_FREE_BY(p_manage->p_read_txt_list, klb_list_destroy);
    KLB_FREE_BY(p_manage->p_read_md_list, klb_list_destroy);
    KLB_FREE_BY(p_manage->p_conn_hlist, klb_hlist_destroy);
    KLB_FREE(p_manage);
}

static em_conn_env_t* em_conn_manage_find(em_conn_manage_t* p_manage, const char* p_name)
{
    em_conn_env_t* p_env = (em_conn_env_t*)klb_hlist_find(p_manage->p_conn_hlist, p_name, strlen(p_name));

    return p_env;
}

int em_conn_manage_connect(em_conn_manage_t* p_manage, const char* p_protocol, const char* p_name, const char* p_url)
{
    assert(NULL != p_manage);

    //LOG("em_conn_manage_connect : [%s],[%s],[%s]", p_protocol, p_name, p_url);

    em_conn_env_t* p_env = KLB_MALLOC(em_conn_env_t, 1, 0);
    KLB_MEMSET(p_env, 0, sizeof(em_conn_env_t));

    if (0 == strcmp(EM_NET_HTTP_FLV, p_protocol))
    {
        // EM_NET_HTTP_FLV
        emfetch_stream_t* p_emfetch = emfetch_stream_create();
        emfetch_flv_t* p_emfetch_flv = emfetch_flv_create(p_name, p_manage, p_emfetch);
        emfetch_flv_init_env(p_env, p_emfetch_flv);

        int name_len = strlen(p_name);
        int ret = klb_hlist_push_tail(p_manage->p_conn_hlist, p_name, name_len, p_env);
        assert(0 == ret);

        emfetch_stream_connect(p_emfetch, "GET", p_url);
    }
    else if (0 == strcmp(EM_NET_HTTP_MNP, p_protocol))
    {
        // EM_NET_HTTP_MNP
        emfetch_stream_t* p_emfetch = emfetch_stream_create();
        emfetch_mnp_t* p_emfetch_mnp = emfetch_mnp_create(p_name, p_manage, p_emfetch);
        emfetch_mnp_init_env(p_env, p_emfetch_mnp);

        int name_len = strlen(p_name);
        int ret = klb_hlist_push_tail(p_manage->p_conn_hlist, p_name, name_len, p_env);
        assert(0 == ret);

        emfetch_stream_connect(p_emfetch, "GET", p_url);
    }
    else if (0 == strcmp(EM_NET_WS_FLV, p_protocol))
    {
        // EM_NET_WS_FLV
        emws_socket_t* p_emws = emws_socket_create();
        emws_flv_t* p_wsflv = emws_flv_create(p_name, p_manage, p_emws);
        emws_flv_init_env(p_env, p_wsflv);

        int name_len = strlen(p_name);
        int ret = em_socket_manage_push_ws(p_manage->p_socket_manage, p_name, name_len, p_emws);
        assert(0 == ret);

        ret = klb_hlist_push_tail(p_manage->p_conn_hlist, p_name, name_len, p_env);
        assert(0 == ret);

        emws_socket_connect(p_emws, p_url);
    }
    else
    {
        // EM_NET_WS_MNP
        emws_socket_t* p_emws = emws_socket_create();
        emws_mnp_t* p_wsmnp = emws_mnp_create(p_name, p_manage, p_emws);
        emws_mnp_init_env(p_env, p_wsmnp);

        int name_len = strlen(p_name);
        int ret = em_socket_manage_push_ws(p_manage->p_socket_manage, p_name, name_len, p_emws);
        assert(0 == ret);

        ret = klb_hlist_push_tail(p_manage->p_conn_hlist, p_name, name_len, p_env);
        assert(0 == ret);

        emws_socket_connect(p_emws, p_url);
    }

    return 0;
}

int em_conn_manage_close(em_conn_manage_t* p_manage, const char* p_name)
{
    assert(NULL != p_manage);
    int name_len = strlen(p_name);

    klb_hlist_iter_t* p_iter = klb_hlist_find_iter(p_manage->p_conn_hlist, p_name, name_len);
    if (NULL != p_iter)
    {
        em_socket_manage_remove(p_manage->p_socket_manage, p_name, name_len);

        em_conn_env_t* p_env = (em_conn_env_t*)klb_hlist_remove(p_manage->p_conn_hlist, p_iter);

        p_env->cb_destroy(p_env->p_conn);
        KLB_FREE(p_env);
    }

    return 1;
}

int em_conn_manage_send(em_conn_manage_t* p_manage, const char* p_name, em_buf_t* p_buf)
{
    assert(NULL != p_manage);

    em_conn_env_t* p_env = em_conn_manage_find(p_manage, p_name);

    if (NULL != p_env && NULL != p_env->cb_send)
    {
        p_env->cb_send(p_env->p_conn, p_buf);
    }

    em_buf_unref_next(p_buf);

    return 0;
}

int em_conn_manage_send_txt(em_conn_manage_t* p_manage, const char* p_name, const char* p_data)
{
    em_buf_t* p_buf = em_buf_mnp_pack_txt(p_data, 0, 0);

    return em_conn_manage_send(p_manage, p_name, p_buf);
}


int em_conn_manage_recv(em_conn_manage_t* p_manage, char* p_protocol, char* p_name, em_buf_t** p_buf, int* p_code)
{
    assert(NULL != p_manage);

    em_conn_manage_packet_txt_t* p_pack_txt = (em_conn_manage_packet_txt_t*)klb_list_pop_head(p_manage->p_read_txt_list);
    if (NULL != p_pack_txt)
    {
        *p_buf = p_pack_txt->p_buf;

        if (NULL != p_protocol)
        {
            strncpy(p_protocol, p_pack_txt->protocol, EM_NET_NAME_LEN);
        }

        if (NULL != p_name)
        {
            strncpy(p_name, p_pack_txt->name, EM_NET_NAME_LEN);
        }

        if (NULL != p_code)
        {
            *p_code = p_pack_txt->code;
        }

        KLB_FREE(p_pack_txt);
        return 0;
    }

    return 1;
}

int em_conn_manage_recv_md(em_conn_manage_t* p_manage, char* p_protocol, char* p_name, em_buf_t** p_buf)
{
    assert(NULL != p_manage);

    em_conn_manage_packet_md_t* p_pack_md = (em_conn_manage_packet_md_t*)klb_list_pop_head(p_manage->p_read_md_list);
    if (NULL != p_pack_md)
    {
        *p_buf = p_pack_md->p_buf;

        if (NULL != p_protocol)
        {
            strncpy(p_protocol, p_pack_md->protocol, EM_NET_NAME_LEN);
        }

        if (NULL != p_name)
        {
            strncpy(p_name, p_pack_md->name, EM_NET_NAME_LEN);
        }

        KLB_FREE(p_pack_md);
        return 0;
    }

    return 1;
}

//int em_conn_manage_send_md(em_conn_manage_t* p_manage)
//{
//    assert(NULL != p_manage);
//
//    return 0;
//}
