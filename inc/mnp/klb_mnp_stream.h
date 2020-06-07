#ifndef __KLB_MNP_STREAM_H__
#define __KLB_MNP_STREAM_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif


/// @def   KLB_MNP_PACK_MAX
/// @brief pack最大数目
#define KLB_MNP_PACK_MAX            32


/// @struct klb_mnp_pack_t
/// @brief  媒体数据
typedef struct klb_mnp_pack_t_
{
    uintptr_t       phy;                        ///< 媒体数据物理地址
    char*           p_data;                     ///< 媒体数据
    int             data_len;                   ///< 数据长度
}klb_mnp_pack_t;


/// @struct klb_mnp_stream_t
/// @brief  码流数据
///  \n go对指针偏移不方便, packs采用固定大小数组
///  \n 附加包: packs[0:extra_num]
///  \n 数据包: packs[extra_num:extra_num+pack_num]
typedef struct klb_mnp_stream_t_
{
    uint32_t        dtype;                      ///< 媒体数据类型: klb_mnp_dtype_e
    
    uint8_t         vtype;                      ///< 视频(video type): klb_mnp_vtype_e
    uint8_t         resv1;
    uint16_t        resv2;

    int64_t         time;                       ///< 时间戳

    int             pack_num;                   ///< 数据包数目
    klb_mnp_pack_t  packs[KLB_MNP_PACK_MAX];    ///< 所有包
}klb_mnp_stream_t;


/// @brief 码流回调函数定义
/// @param [in] chnn        通道
/// @param [in] sidx        流序号: klb_mnp_sidx_e
/// @param [in] *p_stream   码流数据
/// @return int 0.成功; 非0.失败
typedef int(*klb_mnp_stream_cb)(uint32_t chnn, uint32_t sidx, klb_mnp_stream_t* p_stream);


#ifdef __cplusplus
}
#endif

#endif // __KLB_MNP_STREAM_H__
//end
