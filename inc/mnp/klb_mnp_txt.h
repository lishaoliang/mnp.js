///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//  Created: 2019/07/06
//
/// @file    klb_mnp_txt.h
/// @brief   media net protocol, txt定义
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @version 0.1
/// @history 修改历史
///  \n 2019/07/06 0.1 创建文件
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __KLB_MNP_TXT_H__
#define __KLB_MNP_TXT_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif


#pragma pack(4)

/// @struct klb_mnp_txt_t
/// @brief  media net protocol, text head
///  \n F包: [klb_mnp_t][klb_mnp_txt_t][extra][data...]
///  \n B包: [klb_mnp_t][klb_mnp_txt_t][extra][data...]
///  \n C包: [klb_mnp_t][data...]
///  \n E包: [klb_mnp_t][data...]
typedef struct klb_mnp_txt_t_
{
    uint32_t    size;       ///< 完整数据长度(data size, 包含本结构体)
    uint32_t    extra;      ///< 附加数据长度; 正式数据长度 = size - extra - sizeof(klb_mnp_txt_t)
    uint32_t    sequence;   ///< 序列号
    uint32_t    uid;        ///< 用户自定义ID(user defined id)
    // - 4 + 4 + 4 + 4 = 16 Byte
}klb_mnp_txt_t;

#pragma pack()


#ifdef __cplusplus
}
#endif

#endif // __KLB_MNP_TXT_H__
//end
