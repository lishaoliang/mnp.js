///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//
/// @file    klb_atomic.c
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @brief   原子变量
///////////////////////////////////////////////////////////////////////////
#include "thread/klb_atomic.h"
#include "mem/klb_mem.h"
#include <assert.h>

#if !defined(__STM32__) && (!defined(__EMSCRIPTEN__) || (defined(__EMSCRIPTEN__) && defined(__EMSCRIPTEN_PTHREADS__)))
//#error "Unknown compiler klb_atomic.c"

/// @struct klb_atomic_t
/// @brief  原子变量
typedef struct klb_atomic_t_
{
    int volatile    count;      ///< 原子变量地址必须为对齐的地址
}klb_atomic_t;


klb_atomic_t* klb_atomic_create()
{
    klb_atomic_t* p_atomic = KLB_MALLOC_ALIGNED(klb_atomic_t, 1, 0, sizeof(void*));
    KLB_MEMSET(p_atomic, 0, sizeof(klb_atomic_t));

    return p_atomic;
}

void klb_atomic_destroy(klb_atomic_t* p_atomic)
{
    assert(NULL != p_atomic);
    KLB_FREE_ALIGNED(p_atomic);
}

#if defined(_WIN32)
#include <windows.h>

int klb_atomic_ref(klb_atomic_t* p_atomic)
{
    assert(NULL != p_atomic);

    return InterlockedExchangeAdd(&p_atomic->count, 1);
}

int klb_atomic_unref(klb_atomic_t* p_atomic)
{
    assert(NULL != p_atomic);

    return InterlockedExchangeAdd(&p_atomic->count, -1);
}

int klb_atomic_is_zero(klb_atomic_t* p_atomic)
{
    assert(NULL != p_atomic);

    // 为0时, 返回0
    if (0 == InterlockedCompareExchange(&p_atomic->count, 0, 0))
    {
        return 0;
    }

    return 1;
}

#else

int klb_atomic_ref(klb_atomic_t* p_atomic)
{
    assert(NULL != p_atomic);

    return __sync_fetch_and_add(&p_atomic->count, 1);
}

int klb_atomic_unref(klb_atomic_t* p_atomic)
{
    assert(NULL != p_atomic);

    return __sync_fetch_and_sub(&p_atomic->count, 1);
}

int klb_atomic_is_zero(klb_atomic_t* p_atomic)
{
    assert(NULL != p_atomic);

    if (__sync_bool_compare_and_swap(&p_atomic->count, 0, 0))
    {
        return 0;
    }

    return 1;
}

#endif

#endif // #if !defined(__STM32__) && (!defined(__EMSCRIPTEN__) || (defined(__EMSCRIPTEN__) && defined(__EMSCRIPTEN_PTHREADS__)))
