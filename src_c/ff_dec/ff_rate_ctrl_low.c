#include "ff_rate_ctrl.h"
#include "ff_dec_single.h"
#include "mem/klb_mem.h"
#include "mnp/klb_mnp.h"
#include "em_log.h"
#include <assert.h>


/*
 * 场景: 实时流, 许可丢帧
 * 极低延时: 局域网,高速网络
 * 缓存: [7-15]帧
 * 延时: [231-500]毫秒/30fps
 * 抗抖动: [-99, 165]毫秒/30fps
*/


#define FF_RATE_CTRL_LOW_FRAME_MAX        60


static em_frame_yuv_wav_t* ff_rate_ctrl_low_pop_frame(ff_rate_ctrl_t* p_rate_ctrl, int64_t now_ticks)
{
    ff_dec_single_t* p_ff_dec = p_rate_ctrl->p_ff_dec;

    em_frame_yuv_wav_t* p_out = (em_frame_yuv_wav_t*)klb_list_pop_head(p_ff_dec->p_yuv_list);
    if (NULL != p_out && 0 != p_out->pts)
    {
        p_rate_ctrl->last_ticks = now_ticks;
        p_rate_ctrl->last_pts = p_out->pts;
    }

    return p_out;
}

static em_frame_yuv_wav_t* ff_rate_ctrl_low_get_frame(ff_rate_ctrl_t* p_rate_ctrl, int64_t now_ticks)
{
    assert(NULL != p_rate_ctrl);
    ff_dec_single_t* p_ff_dec = p_rate_ctrl->p_ff_dec;
    assert(NULL != p_ff_dec);

    // 有音频帧, 直接给
    if (0 < klb_list_size(p_ff_dec->p_wav_list))
    {
        return (em_frame_yuv_wav_t*)klb_list_pop_head(p_ff_dec->p_wav_list);
    }

    int yuv_size = klb_list_size(p_ff_dec->p_yuv_list);
    if (yuv_size <= 0)
    {
        return NULL;
    }

    // YUV头帧
    em_frame_yuv_wav_t* p_yuv_head = (em_frame_yuv_wav_t*)klb_list_head(p_ff_dec->p_yuv_list);
    if (0 == p_yuv_head->pts)
    {
        // 如果YUV的头帧时间戳为0, 则直接取走
        return ff_rate_ctrl_low_pop_frame(p_rate_ctrl, now_ticks);
    }

    int video_size = klb_list_size(p_ff_dec->p_video_list);
    int all_size = yuv_size + video_size;

    if (0 == p_rate_ctrl->base_ticks || 0 == p_rate_ctrl->base_pts)
    {
        if (7 <= all_size)
        {
            FF_RATE_CTRL_LOG("C ff_rate_ctrl_low_get_frame base");
            p_rate_ctrl->base_ticks = now_ticks;
            p_rate_ctrl->base_pts = p_yuv_head->pts;
            p_rate_ctrl->uniform_num = 0;
            p_rate_ctrl->status = FF_RATE_CTRL_STATUS_UNIFORM;
            
            return ff_rate_ctrl_low_pop_frame(p_rate_ctrl, now_ticks);
        }
    }
    else
    {
        // 帧时间戳异常: 时间减小, 跳帧等
        if (p_yuv_head->pts < p_rate_ctrl->last_pts ||
            FF_RATE_CTRL_TIME_MS * 1333 <= ABS_SUB(p_yuv_head->pts, p_rate_ctrl->last_pts))
        {
            if (FF_RATE_CTRL_TIME_MS * 15 <= ABS_SUB(now_ticks, p_rate_ctrl->last_ticks))
            {
                FF_RATE_CTRL_LOG("C ff_rate_ctrl_low_get_frame pts error");

                p_rate_ctrl->base_ticks = now_ticks;
                p_rate_ctrl->base_pts = p_yuv_head->pts;
                p_rate_ctrl->uniform_num = 0;
                p_rate_ctrl->status = FF_RATE_CTRL_STATUS_UNIFORM;

                return ff_rate_ctrl_low_pop_frame(p_rate_ctrl, now_ticks);
            }
        }
        else
        {
            int64_t dt_ticks = ABS_SUB(now_ticks, p_rate_ctrl->base_ticks);
            int64_t dt_pts = ABS_SUB(p_yuv_head->pts, p_rate_ctrl->base_pts);

            if (FF_RATE_CTRL_STATUS_UNIFORM == p_rate_ctrl->status)
            {
                if (15 < all_size)
                {
                    FF_RATE_CTRL_LOG("C ff_rate_ctrl_low_get_frame FF_RATE_CTRL_STATUS_UNIFORM to FF_RATE_CTRL_STATUS_FAST,all_size:[%d]", all_size);
                    p_rate_ctrl->status = FF_RATE_CTRL_STATUS_FAST;
                }

                if (0 <= all_size && all_size < 7)
                {
                    FF_RATE_CTRL_LOG("C ff_rate_ctrl_low_get_frame FF_RATE_CTRL_STATUS_UNIFORM to FF_RATE_CTRL_STATUS_SLOW,all_size:[%d]", all_size);
                    p_rate_ctrl->status = FF_RATE_CTRL_STATUS_SLOW;
                }
            }

            if (FF_RATE_CTRL_STATUS_FAST == p_rate_ctrl->status)
            {
                bool uniform = false;
                double rate = 1.0;

                if (21 <= all_size)
                {
                    rate = 0.6;
                }
                else if (15 < all_size)
                {
                    rate = 0.8;
                }
                else if (10 < all_size)
                {
                    rate = 0.9;
                }
                else if (0 <= all_size)
                {
                    uniform = true;
                }

                dt_pts = rate * dt_pts;

                if (dt_pts <= dt_ticks + FF_RATE_CTRL_TIME_MS * 6)
                {
                    p_rate_ctrl->base_ticks = now_ticks;
                    p_rate_ctrl->base_pts = p_yuv_head->pts;

                    if (uniform)
                    {
                        FF_RATE_CTRL_LOG("C ff_rate_ctrl_low_get_frame FF_RATE_CTRL_STATUS_FAST to FF_RATE_CTRL_STATUS_UNIFORM,all_size:[%d]", all_size);
                        p_rate_ctrl->uniform_num = 0;
                        p_rate_ctrl->status = FF_RATE_CTRL_STATUS_UNIFORM;
                    }

                    return ff_rate_ctrl_low_pop_frame(p_rate_ctrl, now_ticks);
                }
            }
            else if (FF_RATE_CTRL_STATUS_SLOW == p_rate_ctrl->status)
            {
                bool uniform = false;
                double rate = 1.0;

                if (10 <= all_size)
                {
                    uniform = true;
                }
                else if (0 <= all_size)
                {
                    rate = 1.2;
                }

                dt_pts = rate * dt_pts;

                if (dt_pts <= dt_ticks + FF_RATE_CTRL_TIME_MS * 6)
                {
                    p_rate_ctrl->base_ticks = now_ticks;
                    p_rate_ctrl->base_pts = p_yuv_head->pts;

                    if (uniform)
                    {
                        FF_RATE_CTRL_LOG("C ff_rate_ctrl_low_get_frame FF_RATE_CTRL_STATUS_SLOW to FF_RATE_CTRL_STATUS_UNIFORM,all_size:[%d]", all_size);
                        p_rate_ctrl->uniform_num = 0;
                        p_rate_ctrl->status = FF_RATE_CTRL_STATUS_UNIFORM;
                    }

                    return ff_rate_ctrl_low_pop_frame(p_rate_ctrl, now_ticks);
                }
            }
            else
            {
                if (dt_pts <= dt_ticks + FF_RATE_CTRL_TIME_MS * 6)
                {
                    // 平稳播放约10分钟左右, 重置基准点
                    p_rate_ctrl->uniform_num++;
                    if (18000 < p_rate_ctrl->uniform_num)
                    {
                        p_rate_ctrl->base_ticks = now_ticks;
                        p_rate_ctrl->base_pts = p_yuv_head->pts;
                        p_rate_ctrl->uniform_num = 0;
                    }

                    return ff_rate_ctrl_low_pop_frame(p_rate_ctrl, now_ticks);
                }
            }
        }
    }

    return NULL;
}

static int ff_rate_ctrl_low_drop_data(ff_rate_ctrl_t* p_rate_ctrl, bool hidden)
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
        // 如果正常解码,不能消耗掉数据: 例如PC性能低下情况下
        // 此时也需要丢帧
        if (FF_RATE_CTRL_LOW_FRAME_MAX < klb_list_size(p_ff_dec->p_video_list))
        {
            drop_num = ff_rate_ctrl_drop_to_last_key(p_ff_dec);
        }
    }

    if (FF_RATE_CTRL_LOW_FRAME_MAX < klb_list_size(p_ff_dec->p_video_list))
    {
        // 经过丢帧之后, 仍然还有大量的帧, 则全部丢弃
        drop_num += ff_rate_ctrl_drop_all(p_ff_dec);
    }

    if (0 < drop_num)
    {
        // 一旦丢帧, 丢弃所有待解码的音频帧
        while (0 < klb_list_size(p_ff_dec->p_audio_list))
        {
            em_buf_t* p_buf = klb_list_pop_head(p_ff_dec->p_audio_list);
            em_buf_unref_next(p_buf);
        }

        FF_RATE_CTRL_LOG("C ff_rate_ctrl_low_drop_data:[%d]", drop_num);
    }

    return drop_num;
}

static void ff_rate_ctrl_low_reset(ff_rate_ctrl_t* p_rate_ctrl)
{
    p_rate_ctrl->base_ticks = 0;
    p_rate_ctrl->base_pts = 0;

    p_rate_ctrl->last_ticks = 0;
    p_rate_ctrl->last_pts = 0;

    p_rate_ctrl->uniform_num = 0;
    p_rate_ctrl->status = FF_RATE_CTRL_STATUS_UNIFORM;
}

void ff_rate_ctrl_low_init(ff_rate_ctrl_t* p_rate_ctrl)
{
    p_rate_ctrl->get_frame = ff_rate_ctrl_low_get_frame;
    p_rate_ctrl->drop_data = ff_rate_ctrl_low_drop_data;
    p_rate_ctrl->reset = ff_rate_ctrl_low_reset;
}
