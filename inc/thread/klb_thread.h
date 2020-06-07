///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//  Created: 2019/05/25
//
/// @file    klb_thread.h
/// @brief   线程
///  \n 参考: https://github.com/gozfree/gear-lib/blob/master/libthread/libthread.h
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
///  \n 依赖库: -lpthread
/// @version 0.1
/// @history 修改历史
///  \n 2019/05/25 0.1 创建文件
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __KLB_THREAD_H__
#define __KLB_THREAD_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(__STM32__) && (!defined(__EMSCRIPTEN__) || (defined(__EMSCRIPTEN__) && defined(__EMSCRIPTEN_PTHREADS__)))

/// @struct klb_thread_t
/// @brief  线程
typedef struct klb_thread_t_ klb_thread_t;


/// @brief 创建线程后的启动入口回调函数
/// @param [in] *p_obj      传入的对象
/// @param [in] *p_run      是否在运行
/// @return int 返回值
typedef int(*klb_thread_cb)(void* p_obj, int* p_run);


/// @brief 创建并启动线程
/// @param [in] cb_thread   新线程启动入口
/// @param [in] *p_obj      传递给新线程的对象参数
/// @param [in] cpu_idx     让线程运行的CPU序号; -1.自动, 0.第一个CPU, 1.第二个CPU, 以此类推
/// @param [in] *p_name     线程名称(部分平台支持)
/// @return klb_thread_t*   线程对象
KLB_API klb_thread_t* klb_thread_create(klb_thread_cb cb_thread, void* p_obj, int cpu_idx, const char* p_name);


/// @brief 销毁线程
/// @param [in] *p_thread   线程对象
/// @return 无
KLB_API void klb_thread_destroy(klb_thread_t* p_thread);


/// @brief 毫秒级休眠
/// @param [in] ms          毫秒[0 - ‭2,147,483,648‬]
/// @return 无
/// @note 1秒 = 1000毫秒
KLB_API void klb_sleep(uint32_t ms);


/// @brief 纳秒级休眠
/// @param [in] ns          纳秒[0 - 999,999,999]
/// @return 无
/// @note 1秒 = 1000毫秒 = 1000,000,000纳秒
///  \n 本函数可以实现最多999毫秒休眠
KLB_API void klb_sleep_ns(uint32_t ns);

#endif // #if !defined(__STM32__) && (!defined(__EMSCRIPTEN__) || (defined(__EMSCRIPTEN__) && defined(__EMSCRIPTEN_PTHREADS__)))

#ifdef __cplusplus
}
#endif

#endif // __KLB_THREAD_H__
//end
