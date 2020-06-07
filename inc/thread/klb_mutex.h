///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//  Created: 2019/05/25
//
/// @file    klb_mutex.h
/// @brief   互斥量
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @version 0.1
/// @history 修改历史
///  \n 2019/05/25 0.1 创建文件
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __KLB_MUTEX_H__
#define __KLB_MUTEX_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif


#if !defined(__STM32__) && (!defined(__EMSCRIPTEN__) || (defined(__EMSCRIPTEN__) && defined(__EMSCRIPTEN_PTHREADS__)))


/// @struct klb_mutex_t
/// @brief  通用(互斥量)锁
typedef struct klb_mutex_t_ klb_mutex_t;


/// @brief 创建锁
/// @return klb_mutex_t*    锁对象
KLB_API klb_mutex_t* klb_mutex_create();


/// @brief 销毁锁
/// @param [in] *p_mutex    锁对象
/// @return 无
KLB_API void klb_mutex_destroy(klb_mutex_t* p_mutex);


/// @brief 加锁
/// @param [in] *p_mutex    锁对象
/// @return 无
KLB_API void klb_mutex_lock(klb_mutex_t* p_mutex);


/// @brief 尝试加锁
/// @param [in] *p_mutex    锁对象
/// @return int 0.加锁成功; 非0.加锁失败
KLB_API int klb_mutex_trylock(klb_mutex_t* p_mutex);


/// @brief 解锁
/// @param [in] *p_mutex    锁对象
/// @return 无
/// @note 解锁 klb_mutex_lock / klb_mutex_trylock
KLB_API void klb_mutex_unlock(klb_mutex_t* p_mutex);


#endif // #if !defined(__STM32__) && (!defined(__EMSCRIPTEN__) || (defined(__EMSCRIPTEN__) && defined(__EMSCRIPTEN_PTHREADS__)))

#ifdef __cplusplus
}
#endif

#endif // __KLB_MUTEX_H__
//end
