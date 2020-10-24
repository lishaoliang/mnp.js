#include "tmem_flv.h"
#include "em_flv_demux.h"
#include "list/klb_list.h"
#include "mem/klb_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>



typedef struct tmem_flv_t_
{
    klb_list_t*         p_list;
    klb_list_iter_t*    p_iter;
}tmem_flv_t;


tmem_flv_t* tmem_flv_open(const char* p_data, int data_len)
{
    assert(NULL != p_data);

    tmem_flv_t* p_tmem_flv = KLB_MALLOC(tmem_flv_t, 1, 0);
    KLB_MEMSET(p_tmem_flv, 0, sizeof(tmem_flv_t));

    p_tmem_flv->p_list = klb_list_create();

    em_flv_demux_t* p_demux = em_flv_demux_create();
    em_flv_demux_do(p_demux, p_data, data_len, p_tmem_flv->p_list);
    KLB_FREE_BY(p_demux, em_flv_demux_destroy);

    p_tmem_flv->p_iter = klb_list_begin(p_tmem_flv->p_list);

    return p_tmem_flv;
}

void tmem_flv_close(tmem_flv_t* p_tmem_flv)
{
    assert(NULL != p_tmem_flv);

    while (0 < klb_list_size(p_tmem_flv->p_list))
    {
        em_buf_t* p_tmp = klb_list_pop_head(p_tmem_flv->p_list);

        em_buf_unref_next(p_tmp);
    }


    KLB_FREE_BY(p_tmem_flv->p_list, klb_list_destroy);
    KLB_FREE(p_tmem_flv);
}

int tmem_flv_num(tmem_flv_t* p_tmem_flv)
{
    return klb_list_size(p_tmem_flv->p_list);
}

em_buf_t* tmem_flv_get_next(tmem_flv_t* p_tmem_flv)
{
    if (NULL == p_tmem_flv->p_iter)
    {
        p_tmem_flv->p_iter = klb_list_begin(p_tmem_flv->p_list);
    }

    if (NULL != p_tmem_flv->p_iter)
    {
        em_buf_t* p_buf = (em_buf_t*)klb_list_data(p_tmem_flv->p_iter);

        em_buf_t* p_out = em_buf_malloc_ref(p_buf->buf_len);
        memcpy(p_out->p_buf, p_buf->p_buf, p_buf->end);

        p_out->start = p_buf->start;
        p_out->end = p_buf->end;

        p_tmem_flv->p_iter = klb_list_next(p_tmem_flv->p_iter);
        return p_out;
    }

    return NULL;
}
