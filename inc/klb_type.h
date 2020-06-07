///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//  Created: 2019/05/19
//
/// @file    klb_type.h
/// @brief   基础类型,常用宏定义,C99
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @version 0.1
/// @history 修改历史
///  \n 2019/05/19 0.1 创建文件
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __KLB_TYPE_H__
#define __KLB_TYPE_H__


#ifdef _WIN32
    //define something for Windows (32-bit and 64-bit, this part is common)
    #ifdef _WIN64
        //define something for Windows (64-bit only)
    #else
        //define something for Windows (32-bit only)
    #endif
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
        // iOS Simulator
    #elif TARGET_OS_IPHONE
        // iOS device
    #elif TARGET_OS_MAC
        // Other kinds of Mac OS
    #else
        #error "Unknown Apple platform"
    #endif
#elif __linux__
    // linux
#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#elif defined(__STM32__)
    // STM32
#elif defined(__EMSCRIPTEN__)
    // emscripten
#else
    #error "Unknown compiler"
#endif


/// @def   int*_t, uint*_t
/// @brief 引入标准类型定义
/// @note 
///  \n int8_t  int16_t  int32_t  int64_t
///  \n uint8_t uint16_t uint32_t uint64_t
///  \n intptr_t uintptr_t size_t
///  \n bool false true
#include <stdint.h>     /// int*_t uint*_t
#include <stddef.h>     /// intptr_t uintptr_t size_t
#include <stdbool.h>    /// bool false true


/// @def   KLB_API
/// @brief 导出/导入函数
#if defined(_WIN32)
    #if defined(__KLB_BUILD_DLL__) || defined(__KLB_CORE_API__)
        #define KLB_API extern "C" __declspec(dllexport)
    #elif defined(__KLB_USE_DLL__)
        #define KLB_API extern "C" __declspec(dllimport)
    #else
        #define KLB_API
    #endif

    #define EMSCRIPTEN_KEEPALIVE
#elif defined(__EMSCRIPTEN__)
    #ifdef __cplusplus
        #define KLB_API extern "C"
    #else
        #define KLB_API extern
    #endif

    // EMSCRIPTEN_KEEPALIVE
#else
    #ifdef __cplusplus
        #define KLB_API extern "C"
    #else
        #define KLB_API extern
    #endif

    #define EMSCRIPTEN_KEEPALIVE
#endif


/// @def   NULL
/// @brief NULL定义
#ifndef NULL
    #ifdef __cplusplus
        #define NULL 0
    #else
        #define NULL ((void *)0)
    #endif
#endif


/// @def   MAX
/// @brief MAX宏
#ifndef MAX
    #define MAX(A_,B_)    (((A_) > (B_)) ? (A_) : (B_))
#endif


/// @def   MIN
/// @brief MIN宏
#ifndef MIN
    #define MIN(A_,B_)    (((A_) < (B_)) ? (A_) : (B_))
#endif


/// @def   ABS_SUB
/// @brief ABS
#ifndef ABS_SUB
    #define ABS_SUB(A_, B_)     (((A_) < (B_)) ? ((B_) - (A_)) : ((A_) - (B_)))
#endif


#endif // __KLB_TYPE_H__
//end
