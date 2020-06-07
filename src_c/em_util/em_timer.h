///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    em_timer.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __EM_TIMER_H__
#define __EM_TIMER_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif


void em_sleep(uint32_t ms);

uint32_t em_get_ticks();



#ifdef __cplusplus
}
#endif


#endif // __EM_TIMER_H__
//end

