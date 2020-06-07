///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    em_log.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __EM_LOG_H__
#define __EM_LOG_H__

#include "klb_type.h"
#include "emscripten/emscripten.h"


#if defined(__cplusplus)
extern "C" {
#endif

/// @def   LOG
/// @brief 常规打印
#define LOG(FMT_, ...)      emscripten_log(EM_LOG_CONSOLE, FMT_, ## __VA_ARGS__)


/// @def   LOG_D
/// @brief 调试打印
#define LOG_D(FMT_, ...)    emscripten_log(EM_LOG_CONSOLE, FMT_, ## __VA_ARGS__)


/// @def   LOG_E
/// @brief 错误打印
#define LOG_E(FMT_, ...)    emscripten_log(EM_LOG_WARN, FMT_, ## __VA_ARGS__)


/// @def   LOG_S
/// @brief 系统错误打印
#define LOG_S(FMT_, ...)    emscripten_log(EM_LOG_ERROR, FMT_, ## __VA_ARGS__)


#ifdef __cplusplus
}
#endif

#endif // __EM_LOG_H__
//end
