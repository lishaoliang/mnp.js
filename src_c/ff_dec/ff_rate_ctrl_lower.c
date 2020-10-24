#include "ff_rate_ctrl.h"
#include "ff_dec_single.h"
#include "mem/klb_mem.h"
#include "mnp/klb_mnp.h"
#include <assert.h>


/*
 * 场景: 实时流, 许可丢帧
 * 极低延时: 局域网,高速网络
 * 缓存: [4-10]帧
 * 延时: [231-500]毫秒/30fps
 * 抗抖动: 264毫秒/30fps, [8*(1000/30)]
*/


#define FF_RATE_CTRL_LOWER_FRAME_MAX        40


static em_frame_yuv_wav_t* ff_rate_ctrl_lower_get_frame(ff_rate_ctrl_t* p_rate_ctrl, uint32_t now_ticks)
{
    assert(NULL != p_rate_ctrl);
    ff_dec_single_t* p_ff_dec = p_rate_ctrl->p_ff_dec;
    assert(NULL != p_ff_dec);

    return NULL;
}

static int ff_rate_ctrl_lower_drop_data(ff_rate_ctrl_t* p_rate_ctrl, bool hidden)
{
    assert(NULL != p_rate_ctrl);
    ff_dec_single_t* p_ff_dec = p_rate_ctrl->p_ff_dec;
    assert(NULL != p_ff_dec);

    int drop_num = 0;

    if (hidden)
    {
        drop_num = ff_rate_ctrl_drop_to_last_key(p_ff_dec);
    }
    else
    {
        // 如果正常解码,显示不能消耗掉数据: 例如PC性能低下情况下
        // 此时也需要丢帧
        if (FF_RATE_CTRL_LOWER_FRAME_MAX < klb_list_size(p_ff_dec->p_video_list))
        {
            drop_num = ff_rate_ctrl_drop_to_last_key(p_ff_dec);
        }
    }

    if (FF_RATE_CTRL_LOWER_FRAME_MAX < klb_list_size(p_ff_dec->p_video_list))
    {
        // 经过丢帧之后, 仍然还有大量的帧, 则全部丢弃
        drop_num += ff_rate_ctrl_drop_all(p_ff_dec);
    }

    return drop_num;
}

static void ff_rate_ctrl_lower_reset(ff_rate_ctrl_t* p_rate_ctrl)
{

}

void ff_rate_ctrl_lower_init(ff_rate_ctrl_t* p_rate_ctrl)
{
    p_rate_ctrl->get_frame = ff_rate_ctrl_lower_get_frame;
    p_rate_ctrl->drop_data = ff_rate_ctrl_lower_drop_data;
    p_rate_ctrl->reset = ff_rate_ctrl_lower_reset;
}
