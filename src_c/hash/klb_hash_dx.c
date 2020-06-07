///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//
/// @file    klb_hash_dx.c
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @brief   hash dx算法
///////////////////////////////////////////////////////////////////////////
#include "hash/klb_hash_dx.h"


// 修改自: https://github.com/torvalds/linux/blob/v5.1-rc7/fs/ext4/hash.c
// SPDX-License-Identifier: GPL-2.0
/*
*  linux/fs/ext4/hash.c
*
* Copyright (C) 2002 by Theodore Ts'o
*/


// 原始代码: line 101 ~ line 106
/*
static __u32 dx_hack_hash_unsigned(const char *name, int len)
{
    __u32 hash, hash0 = 0x12a3fe2d, hash1 = 0x37abe8f9;
    const unsigned char *ucp = (const unsigned char *)name;

    while (len--) {
        hash = hash1 + (hash0 ^ (((int)*ucp++) * 7152373));

        if (hash & 0x80000000)
            hash -= 0x7fffffff;
        hash1 = hash0;
        hash0 = hash;
    }
    return hash0 << 1;
}
*/

// 修改后代码
uint32_t klb_hash_dx(const char* p_data, int len)
{
    uint32_t hash, hash0 = 0x12a3fe2d, hash1 = 0x37abe8f9;
    const unsigned char *ucp = (const unsigned char *)p_data;

    while (len--) {
        hash = hash1 + (hash0 ^ (((int)*ucp++) * 7152373));

        if (hash & 0x80000000)
            hash -= 0x7fffffff;
        hash1 = hash0;
        hash0 = hash;
    }
    return hash0 << 1;
}
