///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//  Created: 2019/05/19
//
/// @file    klb_hash_dx.h
/// @brief   hash dx算法
///  \n 修改自: https://github.com/torvalds/linux/blob/v5.1-rc7/fs/ext4/hash.c
///  \n Copyright (C) 2002 by Theodore Ts'o
///  \n linux/fs/ext4/hash.c
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @version 0.1
/// @history 修改历史
///  \n 2019/05/19 0.1 创建文件
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __KLB_HASH_DX_H__
#define __KLB_HASH_DX_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif


/// @brief dx hash unsigned 算法
/// @param [in] *p_data 待计算hash的指针
/// @param [in] len     长度
/// @return uint32_t hash值
KLB_API uint32_t klb_hash_dx(const char* p_data, int len);



#ifdef __cplusplus
}
#endif

#endif // __KLB_HASH_DX_H__
//end
