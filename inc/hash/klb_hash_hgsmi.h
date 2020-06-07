///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//  Created: 2019/06/01
//
/// @file    klb_hash_hgsmi.h
/// @brief   hgsmi哈希算法
///  \n linux内核路径: linux/drivers/gpu/drm/vboxvideo/vbox_hgsmi.c
///  \n 修改自: https://github.com/torvalds/linux/blob/a2d635decbfa9c1e4ae15cb05b68b2559f7f827c/drivers/gpu/drm/vboxvideo/vbox_hgsmi.c
///  \n SPDX-License-Identifier: MIT
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @version 0.1
/// @history 修改历史
///  \n 2019/06/01 0.1 创建文件
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __KLB_HASH_HGSMI_H__
#define __KLB_HASH_HGSMI_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif


/// @brief hgsmi hash 算法
/// @param [in] *p_data 待计算hash的指针
/// @param [in] len     长度
/// @return uint32_t hash值
KLB_API uint32_t klb_hash_hgsmi(const char* p_data, int len);


/// @brief hgsmi hash 算法中间步骤: 可累计计算
/// @param [in] hash    初始值
/// @param [in] *p_data 待计算hash的指针
/// @param [in] len     长度
/// @return uint32_t 中间hash值
KLB_API uint32_t klb_hash_hgsmi_process(uint32_t hash, const char* p_data, int len);


/// @brief hgsmi hash 算法结束得到hash值
/// @param [in] hash    hash值
/// @return uint32_t 最终hash值
///  \n 示例: m1 = klb_hash_hgsmi_process(123, "123456", 6);
///  \n       m2 = klb_hash_hgsmi_process(m1, "789987", 6);
///  \n       h = klb_hash_hgsmi_end(m2);   // 最终得到 "123456789987"的 hash值h
KLB_API uint32_t klb_hash_hgsmi_end(uint32_t hash);


#ifdef __cplusplus
}
#endif

#endif // __KLB_HASH_HGSMI_H__
//end
