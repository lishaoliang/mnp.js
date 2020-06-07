///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//  Created: 2019/05/25
//
/// @file    klb_atomic.h
/// @brief   原子变量
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @version 0.1
/// @history 修改历史
///  \n 2019/05/25 0.1 创建文件
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __KLB_ATOMIC_H__
#define __KLB_ATOMIC_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(__STM32__) && (!defined(__EMSCRIPTEN__) || (defined(__EMSCRIPTEN__) && defined(__EMSCRIPTEN_PTHREADS__)))

/// @struct klb_atomic_t
/// @brief  原子变量
typedef struct klb_atomic_t_ klb_atomic_t;


/// @brief 创建原子变量
/// @return klb_atomic_t*   原子变量
KLB_API klb_atomic_t* klb_atomic_create();


/// @brief 销毁原子变量
/// @param [in] *p_atomic   原子变量
/// @return 无
KLB_API void klb_atomic_destroy(klb_atomic_t* p_atomic);


/// @brief 原子变量计数加1
/// @param [in] *p_atomic   原子变量
/// @return int 返回当前计数
KLB_API int klb_atomic_ref(klb_atomic_t* p_atomic);


/// @brief 原子变量计数减1
/// @param [in] *p_atomic   原子变量
/// @return int 返回当前计数
KLB_API int klb_atomic_unref(klb_atomic_t* p_atomic);


/// @brief 测试原子变量是否为0
/// @param [in] *p_atomic   原子变量
/// @return int 0.表示为0;  非0.表示不为0
KLB_API int klb_atomic_is_zero(klb_atomic_t* p_atomic);

#endif // #if !defined(__STM32__) && (!defined(__EMSCRIPTEN__) || (defined(__EMSCRIPTEN__) && defined(__EMSCRIPTEN_PTHREADS__)))

#ifdef __cplusplus
}
#endif

#endif // __KLB_ATOMIC_H__
//end
