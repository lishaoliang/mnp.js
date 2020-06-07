///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//
/// @file    klb_mem.c
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @brief   基础内存申请/释放
///////////////////////////////////////////////////////////////////////////
#include "mem/klb_mem.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#if !defined(_WIN32) && !defined(__cplusplus)

#if __STDC_VERSION__ < 199901L
#error Not C99
#endif


/// @brief 按对齐申请内存
/// @param [in] size        需要申请的内存大小
/// @param [in] align       对齐系数: 2^N
/// @return void* 申请到的内存
void* _aligned_malloc(size_t size, size_t align)
{
    void* ptr = NULL;
    int ret = posix_memalign(&ptr, align, size);

    assert(NULL != ptr);
    return ptr;
}

#endif
