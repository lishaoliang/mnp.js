#include "em_fetch/em_fetch_stream.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"
#include "libmnpfetch.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


static int emfetch_stream_onopen(void* p_user_data, int code);
static int emfetch_stream_onerror(void* p_user_data, int code);
static int emfetch_stream_onrecv(void* p_user_data, const char* p_data, int data_len);



emfetch_stream_t* emfetch_stream_create()
{
    emfetch_stream_t* p_emfetch = KLB_MALLOC(emfetch_stream_t, 1, 0);
    KLB_MEMSET(p_emfetch, 0, sizeof(emfetch_stream_t));

    return p_emfetch;
}

void emfetch_stream_destroy(emfetch_stream_t* p_emfetch)
{
    assert(NULL != p_emfetch);

    if (0 < p_emfetch->id)
    {
        mnpfetch_close(p_emfetch->id);
        p_emfetch->id = 0;
    }

    KLB_FREE(p_emfetch);
}

int emfetch_stream_connect(emfetch_stream_t* p_emfetch, const char* p_method, const char* p_url)
{
    assert(NULL != p_emfetch);
    

    p_emfetch->status = EMFETCH_STREAM_STATUS_NULL;

    p_emfetch->id = mnpfetch_open(p_url, "", p_emfetch, emfetch_stream_onopen, emfetch_stream_onrecv, emfetch_stream_onerror);

    return 0;
}

//////////////////////////////////////////////////////////////////////////


static int emfetch_stream_onopen(void* p_user_data, int code)
{
    emfetch_stream_t* p_emfetch = (emfetch_stream_t*)p_user_data;
    assert(NULL != p_emfetch);

    p_emfetch->status = EMFETCH_STREAM_STATUS_OPEN;

    // fetch 连接成功
    //LOG("emfetch_stream_onsuccess");

    if (NULL != p_emfetch->cb_open)
    {
        p_emfetch->cb_open(p_emfetch, 0, code);
    }

    return 0;
}

static int emfetch_stream_onerror(void* p_user_data, int code)
{
    emfetch_stream_t* p_emfetch = (emfetch_stream_t*)p_user_data;
    assert(NULL != p_emfetch);
    
    bool open = (EMFETCH_STREAM_STATUS_OPEN == p_emfetch->status) ? true : false;

    p_emfetch->status = EMFETCH_STREAM_STATUS_ERROR;

    // fetch 连接失败
    //LOG("emfetch_stream_onerror, fetch->status:%d", p_fetch->status);

    if (NULL != p_emfetch->cb_error)
    {
        p_emfetch->cb_error(p_emfetch, 0);
    }

    return 0;
}

static int emfetch_stream_onrecv(void* p_user_data, const char* p_data, int data_len)
{
    emfetch_stream_t* p_emfetch = (emfetch_stream_t*)p_user_data;
    assert(NULL != p_emfetch);

    if (NULL != p_emfetch->cb_recv)
    {
        p_emfetch->cb_recv(p_emfetch, 0, p_data, data_len);
    }

    return 0;
}
