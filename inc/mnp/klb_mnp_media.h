///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//  Created: 2019/07/06
//
/// @file    klb_mnp_media.h
/// @brief   media net protocol, media定义
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @version 0.1
/// @history 修改历史
///  \n 2019/07/06 0.1 创建文件
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __KLB_MNP_MEDIA_H__
#define __KLB_MNP_MEDIA_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif


#pragma pack(4)

/// @struct klb_mnp_md_t
/// @brief  media net protocol, media head
///  \n F包: [klb_mnp_t][klb_mnp_md_t][data...]
///  \n B包: [klb_mnp_t][klb_mnp_md_t][data...]
///  \n C包: [klb_mnp_t][data...]
///  \n E包: [klb_mnp_t][data...]
///  \n 数据包可以被存储, 只描述与数据有关部分
typedef struct klb_mnp_md_t_
{
    uint32_t         size;              ///< 完整数据长度(data size, 包含本结构体)
    uint32_t         dtype;             ///< 数据类型(data type): klb_mnp_dtype_e
    //- 4 + 4 = 8 Byte

    uint32_t         chnn;              ///< 通道(channel)
    uint32_t         sidx;              ///< 流序号(stream index): klb_mnp_sidx_e
    //- 8 + 8 = 16 Byte

    int64_t          time;              ///< 时间戳(基于1970年基准,微妙)(范围约:[-292471年, 292471年]
    //- 16 + 8 = 24 Byte

    union
    {
        uint64_t     resv;              ///< 0

        struct
        {
            uint8_t  vtype;             ///< 视频类型(video type): klb_mnp_vtype_e;
        };

        struct
        {
            uint16_t track;             ///< 音频声道数; 1, 2, 5.1;
            uint16_t bit_per_sample;    ///< 音频位宽; 8,16,32
            uint32_t samples;           ///< 音频采样率; 44100
        };
    };
    //- 24 + 8 = 32 Byte
}klb_mnp_md_t;

#pragma pack()

/// @enum   klb_mnp_vtype_e
/// @brief  视频帧类型
typedef enum klb_mnp_vtype_e_
{
    KLB_MNP_VTYPE_I     = 0x00,   ///< I帧
    KLB_MNP_VTYPE_P     = 0x01,   ///< P帧
    KLB_MNP_VTYPE_B     = 0x02,   ///< B帧
    KLB_MNP_VTYPE_MAX   = 0xFF    ///< MAX
}klb_mnp_vtype_e;


/// @enum   klb_mnp_sidx_e
/// @brief  媒体流序号
typedef enum klb_mnp_sidx_e_
{
    KLB_MNP_SIDX_NULL  = 0x0000,    ///< NULL
    KLB_MNP_SIDX_V1    = 0x0001,    ///< Video 1
    KLB_MNP_SIDX_V2    = 0x0002,    ///< Video 2
    KLB_MNP_SIDX_V3    = 0x0003,    ///< Video 3

    KLB_MNP_SIDX_A1    = 0x0021,    ///< Audio 1
    KLB_MNP_SIDX_A2    = 0x0022,    ///< Audio 2
    KLB_MNP_SIDX_A3    = 0x0023,    ///< Audio 3

    KLB_MNP_SIDX_P1    = 0x0041,    ///< Picture 1
    KLB_MNP_SIDX_P2    = 0x0042,    ///< Picture 2
    KLB_MNP_SIDX_P3    = 0x0043,    ///< Picture 3

    KLB_MNP_SIDX_I1    = 0x0061,    ///< Image 1
    KLB_MNP_SIDX_I2    = 0x0062,    ///< Image 2
    KLB_MNP_SIDX_I3    = 0x0063,    ///< Image 3
}klb_mnp_sidx_e;


/// @enum  klb_mnp_dtype_e
/// @brief 媒体数据类型
/// @note 原打算使用ffmpeg的AVCodecID, 但其版本更新过程中值会变更,
///   所以这里重新定义
typedef enum klb_mnp_dtype_e_
{
    KLB_MNP_DTYPE_NULL  = 0x0000,
    KLB_MNP_DTYPE_H264  = 0x0001,   ///< AV_CODEC_ID_H264
    KLB_MNP_DTYPE_H265  = 0x0002,   ///< AV_CODEC_ID_H265

    KLB_MNP_DTYPE_AAC   = 0x1001,   ///< AV_CODEC_ID_AAC

    KLB_MNP_DTYPE_JPEG  = 0x2001,

    KLB_MNP_DTYPE_MAX   = 0x7FFFFFFF,
}klb_mnp_dtype_e;


#ifdef __cplusplus
}
#endif

#endif // __KLB_MNP_MEDIA_H__
//end
