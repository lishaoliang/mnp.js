///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    em_buf.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __EM_BUF_H__
#define __EM_BUF_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct em_buf_t_ em_buf_t;

typedef int(*em_buf_free_cb)(void* p_pool, em_buf_t* p_buf);


typedef struct em_buf_t_
{
    uint32_t        atomic_count;   ///< 引用计数
    uint32_t        atomic_use;     ///< 使用计数

    int             id;             ///< 编号
    int             buf_len;        ///< 缓存长度
    uint8_t*        p_buf;          ///< 缓存

    em_buf_free_cb  cb_free;        ///< 释放缓存
    void*           p_pool;         ///< 内存池

    int             start;          ///< 有效数据起始位置
    int             end;            ///< 有效数据结束位置

    em_buf_t*       p_prev;         ///< 前一节点
    em_buf_t*       p_next;         ///< 后一个节点
}em_buf_t;


em_buf_t* em_buf_malloc_ref(int buf_len);

void em_buf_ref(em_buf_t* p_buf);
void em_buf_ref_next(em_buf_t* p_buf);
void em_buf_ref_all(em_buf_t* p_buf);

void em_buf_unref(em_buf_t* p_buf);
void em_buf_unref_next(em_buf_t* p_buf);
void em_buf_unref_all(em_buf_t* p_buf);

void em_buf_set_pos(em_buf_t* p_buf, int start, int end);

//void em_buf_set_next(em_buf_t* p_buf, em_buf_t* p_v);
void em_buf_set_tail(em_buf_t* p_buf, em_buf_t* p_v);
em_buf_t* em_buf_append(em_buf_t* p_buf, em_buf_t* p_v);

em_buf_t* em_buf_tail(em_buf_t* p_buf);
em_buf_t* em_buf_head(em_buf_t* p_buf);


#ifdef __cplusplus
}
#endif

#endif // __EM_BUF_H__
//end
