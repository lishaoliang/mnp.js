///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//  Created: 2019/05/19
//
/// @file    klb_mem.h
/// @brief   基础内存申请/释放
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @version 0.1
/// @history 修改历史
///   \n 2019/05/19 0.1 创建文件
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __KLB_MEM_H__
#define __KLB_MEM_H__

#include "klb_type.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if defined(__cplusplus)
extern "C" {
#endif


#if !defined(_WIN32)

/// @brief 按对齐申请内存
/// @param [in] size        需要申请的内存大小
/// @param [in] align       对齐系数: 2^N
/// @return void* 申请到的内存
/// @note 返回NULL: align值错误或内存不足
void* _aligned_malloc(size_t size, size_t align);


/// @brief 释放按对齐申请的内存
/// @param [in] *ptr        对齐内存指针
/// @return 无
#define _aligned_free   free

#endif


/// @def   KLB_MALLOC
/// @brief 内存申请; 按结构体数目 + 对齐字节
#define KLB_MALLOC(ST_, NUM_, PADDING_)     (ST_*)malloc(sizeof(ST_) * (NUM_) + (PADDING_))


/// @def   KLB_MEMSET
/// @brief memset, 加入了断言
#define KLB_MEMSET(PTR_, VAL_, SIZE_)       {assert(NULL!=(PTR_));memset(PTR_,VAL_,SIZE_);}


/// @def   KLB_FREE
/// @brief 释放内存, 并将指针置空
#define KLB_FREE(PTR_)                      {if(NULL!=(PTR_)){free(PTR_);(PTR_)=NULL;}}


/// @def   KLB_FREE_BY
/// @brief 通过释放函数释放结构体对象
#define KLB_FREE_BY(PTR_, FUNC_FREE_)       {if(NULL!=(PTR_)){(FUNC_FREE_)(PTR_);(PTR_)=NULL;}}


/// @def   KLB_MALLOC
/// @brief 申请对齐内存; 按结构体数目 + 对齐字节; ALIGN_对齐(一般2^N对齐, 4, 8, 4K等)
#define KLB_MALLOC_ALIGNED(ST_, NUM_, PADDING_, ALIGN_) (ST_*)_aligned_malloc(sizeof(ST_) * (NUM_) + (PADDING_), ALIGN_)


/// @def   KLB_FREE_ALIGNED
/// @brief 释放对齐内存, 并将指针置空
#define KLB_FREE_ALIGNED(PTR_)              {if(NULL!=(PTR_)){_aligned_free(PTR_);(PTR_)=NULL;}}


#ifdef __cplusplus
}
#endif

#endif // __KLB_MEM_H__
//end
