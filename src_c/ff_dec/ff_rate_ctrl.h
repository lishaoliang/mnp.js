#ifndef __FF_RATE_CTRL_H__
#define __FF_RATE_CTRL_H__

#include "klb_type.h"
#include "em_frame_yuv_wav.h"
#include "em_log.h"

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct ff_dec_single_t_ ff_dec_single_t;
typedef struct ff_rate_ctrl_t_ ff_rate_ctrl_t;


#define FF_RATE_CTRL_TIME_MS   ((int64_t)1 * 1000)


#define FF_RATE_CTRL_USE_LOG    0
#if FF_RATE_CTRL_USE_LOG
#define FF_RATE_CTRL_LOG(FMT_, ...)     LOG(FMT_, ## __VA_ARGS__)
#else
#define FF_RATE_CTRL_LOG(FMT_, ...)     
#endif

typedef enum ff_rate_ctrl_type_e_
{
    FF_RATE_CTRL_LOWEST,        // 100毫秒
    FF_RATE_CTRL_LOWER,         // 233毫秒
    FF_RATE_CTRL_LOW,           // 500毫秒
    FF_RATE_CTRL_MIDDLE,        // 1000毫秒
    FF_RATE_CTRL_HIGH,          // 2000毫秒
    FF_RATE_CTRL_HIGHER,        // 3000毫秒

    FF_RATE_CTRL_MAX
}ff_rate_ctrl_type_e;


typedef struct ff_rate_ctrl_t_
{
    /* 帧率控制(流控)
       因素
        抖动来源: 发送方延迟, 网络波动, 浏览器接收数据延误, CPU调度延误等
        误差来源: 有限位数四则运算精度误差等, 误差会被累积
       解决方法
        缓存一定数量的视频帧来抗抖动
        采用类似"弹簧"机制,来消除累积误差,达到连续播放几天,甚至更长
    */
    em_frame_yuv_wav_t* (*get_frame)(ff_rate_ctrl_t* p_rate_ctrl, int64_t now_ticks);


    /* 丢帧策略
        1.丢至最后一个关键帧为止
        2.缓存不能超过一定量: 帧数目, 首尾帧的时间差
    */
    int(*drop_data)(ff_rate_ctrl_t* p_rate_ctrl, bool hidden);


    /* 
        重置参数
    */
    void(*reset)(ff_rate_ctrl_t* p_rate_ctrl);


    ff_dec_single_t*    p_ff_dec;       // 


    int64_t             base_ticks;     // 基准系统滴答数
    int64_t             base_pts;       // 基准YUV图像时间戳
    int64_t             last_ticks;     // 上次取走的系统滴答
    int64_t             last_pts;       // 上次取走的YUV图像时间戳

    int                 status;
#define FF_RATE_CTRL_STATUS_UNIFORM     0
#define FF_RATE_CTRL_STATUS_FAST        1
#define FF_RATE_CTRL_STATUS_SLOW        2

    int                 uniform_num;
}ff_rate_ctrl_t;


ff_rate_ctrl_t* ff_rate_ctrl_create(ff_dec_single_t* p_manage, ff_rate_ctrl_type_e type);
void ff_rate_ctrl_destroy(ff_rate_ctrl_t* p_rate_ctrl);

//////////////////////////////////////////////////////////////////////////

int ff_rate_ctrl_drop_to_last_key(ff_dec_single_t* p_ff_dec);
int ff_rate_ctrl_drop_all(ff_dec_single_t* p_ff_dec);


//////////////////////////////////////////////////////////////////////////
void ff_rate_ctrl_lowest_init(ff_rate_ctrl_t* p_rate_ctrl);
void ff_rate_ctrl_low_init(ff_rate_ctrl_t* p_rate_ctrl);


#ifdef __cplusplus
}
#endif

#endif // __FF_RATE_CTRL_H__
//end
