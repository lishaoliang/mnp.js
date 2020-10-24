#include "ff_rate_ctrl.h"
#include "ff_dec_single.h"
#include "mem/klb_mem.h"
#include "mnp/klb_mnp.h"
#include <assert.h>

ff_rate_ctrl_t* ff_rate_ctrl_create(ff_dec_single_t* p_manage, ff_rate_ctrl_type_e type)
{
    ff_rate_ctrl_t* p_rate_ctrl = KLB_MALLOC(ff_rate_ctrl_t, 1, 0);
    KLB_MEMSET(p_rate_ctrl, 0, sizeof(ff_rate_ctrl_t));

    p_rate_ctrl->p_ff_dec = p_manage;

#if 1
    switch (type)
    {
    case FF_RATE_CTRL_LOWEST:
        ff_rate_ctrl_lowest_init(p_rate_ctrl);
        break;
    case FF_RATE_CTRL_LOW:
        ff_rate_ctrl_low_init(p_rate_ctrl);
        break;
    default:
        ff_rate_ctrl_low_init(p_rate_ctrl);
        break;
    }
#else
    ff_rate_ctrl_lowest_init(p_rate_ctrl);
#endif

    return p_rate_ctrl;
}

void ff_rate_ctrl_destroy(ff_rate_ctrl_t* p_rate_ctrl)
{
    assert(NULL != p_rate_ctrl);
    KLB_FREE(p_rate_ctrl);
}

//////////////////////////////////////////////////////////////////////////


int ff_rate_ctrl_drop_to_last_key(ff_dec_single_t* p_ff_dec)
{
    int drop_num = 0;

    // 丢至最后一个关键帧
    bool drop = false;

    klb_list_iter_t* p_iter = klb_list_end(p_ff_dec->p_video_list);
    while (NULL != p_iter)
    {
        klb_list_iter_t* p_prev = klb_list_prev(p_iter);

        if (drop)
        {
            em_buf_t* p_buf = (em_buf_t*)klb_list_data(p_iter);
            klb_list_remove(p_ff_dec->p_video_list, p_iter);

            em_buf_unref_next(p_buf);
            drop_num++;
        }
        else
        {
            em_buf_t* p_buf = (em_buf_t*)klb_list_data(p_iter);
            klb_mnp_media_t* p_media = (klb_mnp_media_t*)p_buf->p_buf;

            if (KLB_MNP_VTYPE_I == p_media->vtype)
            {
                drop = true;
            }
        }

        p_iter = p_prev;
    }

    return drop_num;
}

int ff_rate_ctrl_drop_all(ff_dec_single_t* p_ff_dec)
{
    int drop_num = 0;

    while (0 < klb_list_size(p_ff_dec->p_video_list))
    {
        em_buf_t* p_buf = (em_buf_t*)klb_list_pop_head(p_ff_dec->p_video_list);

        em_buf_unref_next(p_buf);
        drop_num++;
    }

    return drop_num;
}
