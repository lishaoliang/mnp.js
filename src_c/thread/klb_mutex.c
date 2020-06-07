///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//
/// @file    klb_mutex.c
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @brief   文件简要描述
///////////////////////////////////////////////////////////////////////////
#include "thread/klb_mutex.h"
#include "mem/klb_mem.h"
#include <assert.h>

#if !defined(__STM32__) && (!defined(__EMSCRIPTEN__) || (defined(__EMSCRIPTEN__) && defined(__EMSCRIPTEN_PTHREADS__)))
//#error "Unknown compiler klb_mutex.c"

#ifdef _WIN32
#include <windows.h>

/// @struct klb_mutex_t
/// @brief  通用(互斥量)锁
typedef struct klb_mutex_t_
{
    CRITICAL_SECTION    section;    ///< 互斥量
}klb_mutex_t;

klb_mutex_t* klb_mutex_create()
{
    klb_mutex_t* p_mutex = KLB_MALLOC(klb_mutex_t, 1, 0);
    KLB_MEMSET(p_mutex, 0, sizeof(klb_mutex_t));

    InitializeCriticalSection(&p_mutex->section);

    return p_mutex;
}

void klb_mutex_destroy(klb_mutex_t* p_mutex)
{
    assert(NULL != p_mutex);

    DeleteCriticalSection(&p_mutex->section);
    KLB_FREE(p_mutex);
}

void klb_mutex_lock(klb_mutex_t* p_mutex)
{
    assert(NULL != p_mutex);
    EnterCriticalSection(&p_mutex->section);
}

int klb_mutex_trylock(klb_mutex_t* p_mutex)
{
    assert(NULL != p_mutex);

    if (TryEnterCriticalSection(&p_mutex->section))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void klb_mutex_unlock(klb_mutex_t* p_mutex)
{
    assert(NULL != p_mutex);
    LeaveCriticalSection(&p_mutex->section);
}

#else

#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

#ifndef __APPLE__
#include <sys/prctl.h>
#endif

/// @struct klb_mutex_t
/// @brief  通用(互斥量)锁
typedef struct klb_mutex_t_
{
    pthread_mutex_t     mutex;      ///< 互斥量
}klb_mutex_t;


#ifndef PTHREAD_MUTEX_RECURSIVE
#define PTHREAD_MUTEX_RECURSIVE PTHREAD_MUTEX_RECURSIVE_NP
#endif

klb_mutex_t* klb_mutex_create()
{
    klb_mutex_t* p_mutex = KLB_MALLOC(klb_mutex_t, 1, 0);
    KLB_MEMSET(p_mutex, 0, sizeof(klb_mutex_t));

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&p_mutex->mutex, &attr);
    pthread_mutexattr_destroy(&attr);

    return p_mutex;
}

void klb_mutex_destroy(klb_mutex_t* p_mutex)
{
    assert(NULL != p_mutex);

    pthread_mutex_destroy(&p_mutex->mutex);
    KLB_FREE(p_mutex);
}

void klb_mutex_lock(klb_mutex_t* p_mutex)
{
    assert(NULL != p_mutex);
    pthread_mutex_lock(&p_mutex->mutex);
}

int klb_mutex_trylock(klb_mutex_t* p_mutex)
{
    assert(NULL != p_mutex);

    if (0 == pthread_mutex_trylock(&p_mutex->mutex))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void klb_mutex_unlock(klb_mutex_t* p_mutex)
{
    assert(NULL != p_mutex);
    pthread_mutex_unlock(&p_mutex->mutex);
}

#endif

#endif // #if !defined(__STM32__) && (!defined(__EMSCRIPTEN__) || (defined(__EMSCRIPTEN__) && defined(__EMSCRIPTEN_PTHREADS__)))
