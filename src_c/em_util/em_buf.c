#include "em_buf.h"
#include "mem/klb_mem.h"
#include <assert.h>

#ifdef __EMSCRIPTEN_PTHREADS__

#include "emscripten/threading.h"

// 使用__EMSCRIPTEN_PTHREADS__, 编译提示
// wasm::Asm2WasmBuilder::processFunction(cashew::Ref)::<lambda(cashew::Ref)>: Assertion `mappedGlobals.find(name) != mappedGlobals.end() ? true : (std::cerr << name.str << '\n', false)' failed
// 
// 需要使用编译选项:
// MY_CFLAGS += -s USE_PTHREADS=1
// MY_CFLAGS += -s PTHREAD_POOL_SIZE = 4

#else

static uint32_t emscripten_atomic_and_u32(void/*uint32_t*/ *addr, uint32_t val)
{
    uint32_t* ptr = (uint32_t*)addr;
    *ptr += val;
    return *ptr;
}

static uint32_t emscripten_atomic_store_u32(void/*uint32_t*/ *addr, uint32_t val)
{
    uint32_t* ptr = (uint32_t*)addr;
    uint32_t ret = *ptr;

    *ptr = val;

    return ret;
}

#endif


static int cb_em_buf_free(void* p_pool, em_buf_t* p_buf)
{
    assert(NULL == p_pool);
    assert(NULL != p_buf);
    // 非内存申池方式申请, 这里直接释放

    KLB_FREE(p_buf->p_buf);
    KLB_FREE(p_buf);

    return 0;
}

em_buf_t* em_buf_malloc_ref(int buf_len)
{
    assert(0 < buf_len);

    em_buf_t* p_buf = KLB_MALLOC(em_buf_t, 1, 0);
    KLB_MEMSET(p_buf, 0, sizeof(em_buf_t));

    p_buf->buf_len = buf_len;
    p_buf->p_buf = KLB_MALLOC(uint8_t, buf_len, 0);
    KLB_MEMSET(p_buf->p_buf, 0, buf_len);

    p_buf->cb_free = cb_em_buf_free;

    em_buf_ref(p_buf);
    return p_buf;
}

void em_buf_ref(em_buf_t* p_buf)
{
    assert(NULL != p_buf);

    emscripten_atomic_and_u32(&p_buf->atomic_count, 1);
}

void em_buf_ref_next(em_buf_t* p_buf)
{
    assert(NULL != p_buf);
    em_buf_t* p_cur = p_buf;

    while (NULL != p_cur)
    {
        em_buf_t* p_next = p_cur->p_next;
        em_buf_ref(p_cur);
        p_cur = p_next;
    }
}

void em_buf_ref_all(em_buf_t* p_buf)
{
    assert(NULL != p_buf);
    em_buf_t* p_cur = p_buf->p_prev;

    while (NULL != p_cur)
    {
        em_buf_t* p_prev = p_cur->p_prev;
        em_buf_ref(p_cur);
        p_cur = p_prev;
    }

    em_buf_ref_next(p_buf);
}

void em_buf_unref(em_buf_t* p_buf)
{
    assert(NULL != p_buf);
    uint32_t n = emscripten_atomic_and_u32(&p_buf->atomic_count, -1);

    if (0 == n)
    {
        p_buf->start = 0;
        p_buf->end = 0;

        p_buf->p_prev = NULL;
        p_buf->p_next = NULL;

        emscripten_atomic_store_u32(&p_buf->atomic_count, 0);
        emscripten_atomic_store_u32(&p_buf->atomic_use, 0);

        em_buf_free_cb cb_free = p_buf->cb_free;
        assert(NULL != cb_free);
        cb_free(p_buf->p_pool, p_buf);
    }
}

void em_buf_unref_next(em_buf_t* p_buf)
{
    assert(NULL != p_buf);

    assert(NULL != p_buf);
    em_buf_t* p_cur = p_buf;

    while (NULL != p_cur)
    {
        em_buf_t* p_next = p_cur->p_next;
        em_buf_unref(p_cur);
        p_cur = p_next;
    }
}

void em_buf_unref_all(em_buf_t* p_buf)
{
    assert(NULL != p_buf);
    em_buf_t* p_cur = p_buf->p_prev;

    while (NULL != p_cur)
    {
        em_buf_t* p_prev = p_cur->p_prev;
        em_buf_unref(p_cur);
        p_cur = p_prev;
    }

    em_buf_unref_next(p_buf);
}

void em_buf_set_pos(em_buf_t* p_buf, int start, int end)
{
    assert(NULL != p_buf);
    assert(start <= p_buf->buf_len);
    assert(end <= p_buf->buf_len);
    assert(start <= end);

    p_buf->start = MIN(start, p_buf->buf_len);
    p_buf->end = MIN(end, p_buf->buf_len);
}

//void em_buf_set_next(em_buf_t* p_buf, em_buf_t* p_v)
//{
//    assert(NULL != p_buf);
//}

void em_buf_set_tail(em_buf_t* p_buf, em_buf_t* p_v)
{
    assert(NULL != p_buf);

    em_buf_t* p_tail = em_buf_tail(p_buf);

    p_tail->p_next = p_v;
    if (NULL != p_v)
    {
        p_v->p_prev = p_tail;
    }
}

em_buf_t* em_buf_append(em_buf_t* p_buf, em_buf_t* p_v)
{
    if (NULL == p_buf)
    {
        return p_v;
    }
    else
    {
        em_buf_t* p_head = em_buf_head(p_buf);
        em_buf_set_tail(p_head, p_v);

        return p_head;
    }
}

em_buf_t* em_buf_tail(em_buf_t* p_buf)
{
    assert(NULL != p_buf);

    em_buf_t* p_cur = p_buf;
    while (NULL != p_cur->p_next)
    {
        p_cur = p_cur->p_next;
    }

    return p_cur;
}

em_buf_t* em_buf_head(em_buf_t* p_buf)
{
    assert(NULL != p_buf);

    em_buf_t* p_cur = p_buf;
    while (NULL != p_cur->p_prev)
    {
        p_cur = p_cur->p_prev;
    }

    return p_cur;
}

