///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//
/// @file    klb_thread.c
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @brief   文件简要描述
///////////////////////////////////////////////////////////////////////////
#include "thread/klb_thread.h"
#include "mem/klb_mem.h"
#include "log/klb_log.h"
#include <assert.h>

#if !defined(__STM32__) && (!defined(__EMSCRIPTEN__) || (defined(__EMSCRIPTEN__) && defined(__EMSCRIPTEN_PTHREADS__)))
//#error "Unknown compiler klb_thread.c"

#ifdef _WIN32
#include <windows.h>


/// @struct klb_thread_t
/// @brief  线程对象
typedef struct klb_thread_t_
{
    HANDLE          hnd;            ///< 线程句柄
    int             run;            ///< 是否运行: TRUE.在运行; FALSE.未运行

    klb_thread_cb   cb_thread;      ///< 线程启动入口
    void*           p_obj;          ///< 传递的对象参数

    int             cpu_idx;        ///< 需要设置CPU序号
    char*           p_name;         ///< 需要设置的线程名称
}klb_thread_t;


static DWORD WINAPI cb_klb_thread(void* p_obj)
{
    klb_thread_t* p_thread = (klb_thread_t*)(p_obj);

    if (NULL != p_thread->cb_thread)
    {
        p_thread->cb_thread(p_thread->p_obj, &p_thread->run);
    }

    return 0;
}

klb_thread_t* klb_thread_create(klb_thread_cb cb_thread, void* p_obj, int cpu_idx, const char* p_name)
{
    assert(NULL != cb_thread);

    klb_thread_t* p_thread = KLB_MALLOC(klb_thread_t, 1, 0);
    KLB_MEMSET(p_thread, 0, sizeof(klb_thread_t));

    p_thread->cb_thread = cb_thread;
    p_thread->p_obj = p_obj;
    p_thread->cpu_idx = cpu_idx;

    p_thread->run = true;
    p_thread->hnd = CreateThread(0, 0, cb_klb_thread, (void *)p_thread, 0, NULL);

    if (NULL == p_thread->hnd)
    {
        KLB_LOG_S("sys error!klb thread create!");

        KLB_FREE(p_thread->p_name);
        KLB_FREE(p_thread);
        return NULL;
    }

    return p_thread;
}


void klb_thread_destroy(klb_thread_t* p_thread)
{
    assert(NULL != p_thread);

    p_thread->run = false;
    WaitForSingleObject(p_thread->hnd, INFINITE);
    CloseHandle(p_thread->hnd);

    KLB_FREE(p_thread->p_name);
    KLB_FREE(p_thread);
}

void klb_sleep(uint32_t ms)
{
    Sleep(ms);
}

void klb_sleep_ns(uint32_t ns)
{
    assert(ns <= 999999999);
    assert(false);
}

#else

#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>

#ifndef __APPLE__
#include <sys/prctl.h>
#endif

/// @struct klb_thread_t
/// @brief  线程对象
typedef struct klb_thread_t_
{
    pthread_t       hnd;            ///< 线程句柄
    int             run;            ///< 是否运行: TRUE.在运行; FALSE.未运行

    klb_thread_cb   cb_thread;      ///< 线程启动入口
    void*           p_obj;          ///< 传递的对象参数

    int             cpu_idx;        ///< 需要设置CPU序号
    char*           p_name;         ///< 需要设置的线程名称
}klb_thread_t;


static void* cb_klb_thread(void* p_obj)
{
    klb_thread_t* p_thread = (klb_thread_t*)(p_obj);

#if !defined(__APPLE__) && !defined(__EMSCRIPTEN__)
    // 设置CPU
    if (0 <= p_thread->cpu_idx)
    {
        cpu_set_t mask;

        CPU_ZERO(&mask);
        CPU_SET(p_thread->cpu_idx, &mask);

        int tid = syscall(__NR_gettid);;
        if (-1 == sched_setaffinity(tid, sizeof(mask), &mask))
        {
            LOG_E("error!klb thread set cpu idx:%d\n", p_thread->cpu_idx);
        }
    }

    // 设置线程名称
    if (NULL != p_thread->p_name)
    {
        prctl(PR_SET_NAME, p_thread->p_name);
    }
#endif

    if (NULL != p_thread->cb_thread)
    {
        p_thread->cb_thread(p_thread->p_obj, &p_thread->run);
    }

    return 0;
}

klb_thread_t* klb_thread_create(klb_thread_cb cb_thread, void* p_obj, int cpu_idx, const char* p_name)
{
    assert(NULL != cb_thread);

    klb_thread_t* p_thread = KLB_MALLOC(klb_thread_t, 1, 0);
    KLB_MEMSET(p_thread, 0, sizeof(klb_thread_t));

    p_thread->cb_thread = cb_thread;
    p_thread->p_obj = p_obj;
    p_thread->cpu_idx = cpu_idx;

    p_thread->run = true;

    if (0 != pthread_create(&(p_thread->hnd), 0, cb_klb_thread, p_thread))
    {
        LOG_S("sys error!klb thread create!");

        KLB_FREE(p_thread->p_name);
        KLB_FREE(p_thread);
        return NULL;
    }

    return p_thread;
}

void klb_thread_destroy(klb_thread_t* p_thread)
{
    assert(NULL != p_thread);

    p_thread->run = false;
    pthread_join(p_thread->hnd, NULL);

    KLB_FREE(p_thread->p_name);
    KLB_FREE(p_thread);
}

void klb_sleep(uint32_t ms)
{
    if (ms <= 10)
    {
        // [0毫秒, 10毫秒]采用纳秒级休眠
        klb_sleep_ns(ms * 1000000);
    }
    else if (ms <= 60000)
    {
        // (10毫秒, 60秒]采用 usleep
        usleep(ms * 1000);
    }
    else
    {
        // (60秒,N] 60秒以上, 忽略秒以下休眠时间
        uint32_t s = ms / 1000;

        do
        {
            int left = sleep(s);

            if (left < s)
            {
                s = left;
            }
            else
            {
                s = 0;
            }

        } while (0 < s);
    }
}

void klb_sleep_ns(uint32_t ns)
{
    assert(ns <= 999999999);

    struct timespec wait;
    wait.tv_sec = 0;
    wait.tv_nsec = ns;

#if !defined(__APPLE__) && !defined(__EMSCRIPTEN__)
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wait, NULL);
#else
    nanosleep(&wait, NULL);
#endif
}

#endif

#endif //#if !defined(__STM32__) && (!defined(__EMSCRIPTEN__) || (defined(__EMSCRIPTEN__) && defined(__EMSCRIPTEN_PTHREADS__)))
