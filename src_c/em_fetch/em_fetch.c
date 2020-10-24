#include "em_fetch/em_fetch.h"

#if 0

#include "emscripten/fetch.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"
#include "assert.h"

typedef struct mnp_fetch_t_
{
    emscripten_fetch_attr_t     attr;
    emscripten_fetch_t*         p_em_fetch;
}mnp_fetch_t;


static void mnp_fetch_onsuccess(emscripten_fetch_t* p_em_fetch)
{
    mnp_fetch_t* p_fetch = (mnp_fetch_t*)p_em_fetch->userData;

    LOG("onsuccess");
    //emscripten_fetch_close(p_em_fetch);
}

static void mnp_fetch_onerror(emscripten_fetch_t* p_em_fetch)
{
    mnp_fetch_t* p_fetch = (mnp_fetch_t*)p_em_fetch->userData;

    LOG("onerror, fetch->status:%d", p_em_fetch->status);
    //emscripten_fetch_close(p_em_fetch);
}

static void mnp_fetch_onprogress(emscripten_fetch_t* p_em_fetch)
{
    mnp_fetch_t* p_fetch = (mnp_fetch_t*)p_em_fetch->userData;

    //LOG("onprogress", );



    //LOG("Downloading %s.. %.2f%s complete. HTTP readyState: %d. HTTP status: %d.\n"
    //    "HTTP statusText: %s. Received chunk [%llu, %llu]\n",
    //    p_em_fetch->url, (p_em_fetch->totalBytes > 0) ? ((p_em_fetch->dataOffset + p_em_fetch->numBytes) * 100.0 / p_em_fetch->totalBytes) : (double)(p_em_fetch->dataOffset + p_em_fetch->numBytes),
    //    (p_em_fetch->totalBytes > 0) ? "%" : " bytes",
    //    p_em_fetch->readyState, p_em_fetch->status, p_em_fetch->statusText,
    //    p_em_fetch->dataOffset, p_em_fetch->dataOffset + p_em_fetch->numBytes);

    //emscripten_fetch_close(p_em_fetch);
}

mnp_fetch_t* mnp_fetch_create()
{
    mnp_fetch_t* p_fetch = KLB_MALLOC(mnp_fetch_t, 1, 0);
    KLB_MEMSET(p_fetch, 0, sizeof(mnp_fetch_t));

    emscripten_fetch_attr_init(&p_fetch->attr);

    p_fetch->attr.attributes = EMSCRIPTEN_FETCH_STREAM_DATA;
    p_fetch->attr.onsuccess = mnp_fetch_onsuccess;
    p_fetch->attr.onprogress = mnp_fetch_onprogress;
    p_fetch->attr.onerror = mnp_fetch_onerror;
    //p_fetch->attr.timeoutMSecs = 3 * 60;

    return p_fetch;
}

void mnp_fetch_destroy(mnp_fetch_t* p_fetch)
{
    assert(NULL != p_fetch);
    if (NULL != p_fetch)
    {
        LOG("mnp_fetch_destroy");

        KLB_FREE(p_fetch);
        KLB_FREE_BY(p_fetch->p_em_fetch, emscripten_fetch_close);
    }
}

int mnp_fetch_request(mnp_fetch_t* p_fetch, const char* p_method, const char* p_url)
{
    assert(NULL != p_fetch);
    assert(NULL == p_fetch->p_em_fetch);

    strncpy(p_fetch->attr.requestMethod, p_method, sizeof(p_fetch->attr.requestMethod) - 1);
    p_fetch->attr.userData = p_fetch;

    p_fetch->p_em_fetch = emscripten_fetch(&p_fetch->attr, p_url);

    return (NULL == p_fetch->p_em_fetch)? false : true;
}

#endif
