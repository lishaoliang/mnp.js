///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//  Created: 2019/07/06
//
/// @file    klb_mnp.h
/// @brief   media net protocol, 媒体网络协议
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @version 0.1
/// @history 修改历史
///  \n 2019/07/06 0.1 创建文件
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __KLB_MNP_H__
#define __KLB_MNP_H__

#include "klb_type.h"
#include "mnp/klb_mnp_txt.h"
#include "mnp/klb_mnp_bin.h"
#include "mnp/klb_mnp_media.h"
#include "mnp/klb_mnp_stream.h"

#if defined(__cplusplus)
extern "C" {
#endif


#pragma pack(4)

/// @struct klb_mnp_t
/// @brief  网络封包头
///  \n 固定8字节, 封包头可以被写入文件, 需要精简大小
///  \n 缓存大小:
///  \n  媒体包缓存: [4K, 8K, 16K, 32K]
typedef struct klb_mnp_t_
{
    uint32_t magic;             ///< 魔数: KLB_MNP_MAGIC
    //- 4 Byte

    uint16_t size ;             ///< 单个数据包大小(包含本结构体) <= 32K

    uint8_t  opt : 2;           ///< 包组合方式: klb_mnp_opt_e
    uint8_t  packtype : 5;      ///< 包类型: klb_mnp_packtype_e
    uint8_t  resv1 : 1;         ///< 0
    uint8_t  resv2;             ///< 0
    //- 4 + 4 = 8 Byte
}klb_mnp_t;

#pragma pack()


/// @def   KLB_MNP_MAGIC
/// @brief k media net protocol魔数
#define KLB_MNP_MAGIC           0x504E4DEB  ///< "*MNP"


/// @struct klb_mnp_opt_e
/// @brief  包组合方式
typedef enum klb_mnp_opt_e_
{
    KLB_MNP_BEGIN    = 0x0,     ///< Begin包(B包)
    KLB_MNP_CONTINUE = 0x1,     ///< Continue包(C包)
    KLB_MNP_END      = 0x2,     ///< End包(E包)
    KLB_MNP_FULL     = 0x3,     ///< Full包(F包)

    KLB_MNP_OPT_MAX  = 0x3      ///< MAX
}klb_mnp_opt_e;


/// @struct klb_mnp_packtype_e
/// @brief  包类型
typedef enum klb_mnp_packtype_e_
{
    KLB_MNP_HEART        = 0x0, ///< 心跳包: 附加数据为0; 否则协议错误
    KLB_MNP_TXT          = 0x1, ///< 文本数据
    KLB_MNP_BIN          = 0x2, ///< 二进制数据
    KLB_MNP_MEDIA        = 0x3, ///< 媒体数据

    KLB_MNP_PACKTYPE_MAX = 0x1F ///< MAX
}klb_mnp_packtype_e;


#ifdef __cplusplus
}
#endif

#endif // __KLB_MNP_H__
//end
