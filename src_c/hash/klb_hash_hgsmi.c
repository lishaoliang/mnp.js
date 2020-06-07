///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//
/// @file    klb_hash_hgsmi.c
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @brief   hgsmi哈希算法
///////////////////////////////////////////////////////////////////////////
#include "hash/klb_hash_hgsmi.h"


// linux内核路径: linux/drivers/gpu/drm/vboxvideo/vbox_hgsmi.c
// 修改自: https://github.com/torvalds/linux/blob/a2d635decbfa9c1e4ae15cb05b68b2559f7f827c/drivers/gpu/drm/vboxvideo/vbox_hgsmi.c
// SPDX-License-Identifier: MIT
/*
* Copyright (C) 2017 Oracle Corporation
* Authors: Hans de Goede <hdegoede@redhat.com>
*/

/* One-at-a-Time Hash from http://www.burtleburtle.net/bob/hash/doobs.html */
uint32_t klb_hash_hgsmi_process(uint32_t hash, const char* p_data, int len)
{
    const unsigned char* data = (const unsigned char*)p_data;

    while (len--) {
        hash += *data++;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    return hash;
}

uint32_t klb_hash_hgsmi_end(uint32_t hash)
{
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

uint32_t klb_hash_hgsmi(const char* p_data, int len)
{
    uint32_t b = 0x789AFEBA;

    uint32_t h = klb_hash_hgsmi_process(b, (const char*)p_data, len);
    return klb_hash_hgsmi_end(h);
}
