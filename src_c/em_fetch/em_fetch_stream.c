#include "em_fetch/em_fetch_stream.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


static void emfetch_stream_onsuccess(emscripten_fetch_t* p_fetch);
static void emfetch_stream_onerror(emscripten_fetch_t* p_fetch);
static void emfetch_stream_onprogress(emscripten_fetch_t* p_fetch);


emfetch_stream_t* emfetch_stream_create()
{
    emfetch_stream_t* p_emfetch = KLB_MALLOC(emfetch_stream_t, 1, 0);
    KLB_MEMSET(p_emfetch, 0, sizeof(emfetch_stream_t));

    emscripten_fetch_attr_init(&p_emfetch->attr);

    p_emfetch->attr.attributes = EMSCRIPTEN_FETCH_STREAM_DATA;
    p_emfetch->attr.onsuccess = emfetch_stream_onsuccess;
    p_emfetch->attr.onprogress = emfetch_stream_onprogress;
    p_emfetch->attr.onerror = emfetch_stream_onerror;

    //p_emfetch->attr.timeoutMSecs = 2 * 60;

    return p_emfetch;
}

void emfetch_stream_destroy(emfetch_stream_t* p_emfetch)
{
    assert(NULL != p_emfetch);

    KLB_FREE_BY(p_emfetch->p_fetch, emscripten_fetch_close);
    KLB_FREE(p_emfetch);
}

int emfetch_stream_connect(emfetch_stream_t* p_emfetch, const char* p_method, const char* p_ip, int port, const char* p_path)
{
    assert(NULL != p_emfetch);
    assert(NULL == p_emfetch->p_fetch);
    
    char url[516] = { 0 };
    snprintf(url, 512, "http://%s:%d%s", p_ip, port, p_path);
    //const char* p_url = NULL;

    strncpy(p_emfetch->attr.requestMethod, p_method, sizeof(p_emfetch->attr.requestMethod) - 1);
    p_emfetch->attr.userData = p_emfetch;

    p_emfetch->status = EMFETCH_STREAM_STATUS_NULL;
    p_emfetch->p_fetch = emscripten_fetch(&p_emfetch->attr, url);

    return 0;
}

//////////////////////////////////////////////////////////////////////////


static void emfetch_stream_onsuccess(emscripten_fetch_t* p_fetch)
{
    emfetch_stream_t* p_emfetch = (emfetch_stream_t*)p_fetch->userData;
    assert(NULL != p_emfetch);

    p_emfetch->status = EMFETCH_STREAM_STATUS_OPEN;

    // fetch 连接成功
    //LOG("emfetch_stream_onsuccess");

    if (NULL != p_emfetch->cb_open)
    {
        p_emfetch->cb_open(p_emfetch, 0, 0); // 新连接成功
    }
}

static void emfetch_stream_onerror(emscripten_fetch_t* p_fetch)
{
    emfetch_stream_t* p_emfetch = (emfetch_stream_t*)p_fetch->userData;
    assert(NULL != p_emfetch);
    
    bool open = (EMFETCH_STREAM_STATUS_OPEN == p_emfetch->status) ? true : false;

    p_emfetch->status = EMFETCH_STREAM_STATUS_ERROR;

    // fetch 连接失败
    //LOG("emfetch_stream_onerror, fetch->status:%d", p_fetch->status);

    if (open)
    {
        if (NULL != p_emfetch->cb_open)
        {
            p_emfetch->cb_open(p_emfetch, 0, 1); // 新连接失败
        }
    }
    else
    {
        if (NULL != p_emfetch->cb_error)
        {
            p_emfetch->cb_error(p_emfetch, 0);
        }
    }
}

static void emfetch_stream_onprogress(emscripten_fetch_t* p_fetch)
{
    emfetch_stream_t* p_emfetch = (emfetch_stream_t*)p_fetch->userData;
    assert(NULL != p_emfetch);

    //LOG("onprogress", );
    //LOG("Downloading %s.. %.2f%s complete. HTTP readyState: %d. HTTP status: %d.\n"
    //    "HTTP statusText: %s. Received chunk [%llu, %llu]\n",
    //    p_fetch->url, (p_fetch->totalBytes > 0) ? ((p_fetch->dataOffset + p_fetch->numBytes) * 100.0 / p_fetch->totalBytes) : (double)(p_fetch->dataOffset + p_fetch->numBytes),
    //    (p_fetch->totalBytes > 0) ? "%" : " bytes",
    //    p_fetch->readyState, p_fetch->status, p_fetch->statusText,
    //    p_fetch->dataOffset, p_fetch->dataOffset + p_fetch->numBytes);

    if (NULL != p_emfetch->cb_recv)
    {
        p_emfetch->cb_recv(p_emfetch, 0, p_fetch);
    }
}
