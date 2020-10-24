#include "em_ws/em_ws.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"
#include "em_net/em_socket.h"
#include <stdio.h>
#include <assert.h>


static EM_BOOL cb_emws_socket_open(int eventType, const EmscriptenWebSocketOpenEvent *e, void *userData);
static EM_BOOL cb_emws_socket_close(int eventType, const EmscriptenWebSocketCloseEvent *e, void *userData);
static EM_BOOL cb_emws_socket_error(int eventType, const EmscriptenWebSocketErrorEvent *e, void *userData);
static EM_BOOL cb_emws_socket_message(int eventType, const EmscriptenWebSocketMessageEvent *e, void *userData);


emws_socket_t* emws_socket_create()
{
    emws_socket_t* p_emws = KLB_MALLOC(emws_socket_t, 1, 0);
    KLB_MEMSET(p_emws, 0, sizeof(emws_socket_t));

    p_emws->fd = INVALID_SOCKET;
    p_emws->status = EMWS_SOCKET_NULL;

    return p_emws;
}

void emws_socket_destroy(emws_socket_t* p_emws)
{
    assert(NULL != p_emws);

    if (0 < p_emws->fd)
    {
        //LOG("emscripten_websocket_delete:[%d]", p_emws->fd);

        emscripten_websocket_close(p_emws->fd, 0, NULL);
        emscripten_websocket_delete(p_emws->fd);
        p_emws->fd = INVALID_SOCKET;
    }

    KLB_FREE(p_emws);
}

static EMSCRIPTEN_WEBSOCKET_T emws_open(const char* p_url)
{
    EmscriptenWebSocketCreateAttributes attr;
    emscripten_websocket_init_create_attributes(&attr);

    attr.url = p_url;   // eg. "ws://localhost:8000"

    EMSCRIPTEN_WEBSOCKET_T socket = emscripten_websocket_new(&attr);
    //LOG("emscripten_websocket_new:[%d]", socket);

    return socket;
}

int emws_socket_connect(emws_socket_t* p_emws, const char* p_url)
{
    assert(NULL != p_emws);
    assert(NULL != p_url);

    //EMSCRIPTEN_WEBSOCKET_T socket = emws_open("ws://192.168.110.129:8080/stream/wsmnp/*");
    EMSCRIPTEN_WEBSOCKET_T socket = emws_open(p_url);
    if (socket <= 0)
    {
        // 浏览器支持情况下,参数填正确了,一定得到socket
        // 是否成功连接到目标, 则在open/error的回调函数中被通知
        LOG_E("C WebSocket creation failed, error code %d!\n", (EMSCRIPTEN_RESULT)socket);
        return 1;
    }

    p_emws->fd = socket;
    p_emws->status = EMWS_SOCKET_NULL;

    emscripten_websocket_set_onopen_callback(p_emws->fd, (void*)p_emws, cb_emws_socket_open);
    emscripten_websocket_set_onclose_callback(p_emws->fd, (void*)p_emws, cb_emws_socket_close);
    emscripten_websocket_set_onerror_callback(p_emws->fd, (void*)p_emws, cb_emws_socket_error);
    emscripten_websocket_set_onmessage_callback(p_emws->fd, (void*)p_emws, cb_emws_socket_message);

    return 0;
}

int emws_socket_send_utf8_text(emws_socket_t* p_emws, const uint8_t* p_buf, int len)
{
    int ret = emscripten_websocket_send_utf8_text(p_emws->fd, (const char*)p_buf);
    //LOG("emws_socket_send_utf8_text: ret=%d,len=%d", ret, len);

    if (0 == ret)
    {
        return len; // 发送成功
    }

    return 0; // 发送失败
}

int emws_socket_send_binary(emws_socket_t* p_emws, const uint8_t* p_buf, int len)
{
    int ret = emscripten_websocket_send_binary(p_emws->fd, (void*)p_buf, len);

    if (0 == ret)
    {
        return len; // 发送成功
    }

    return 0; // 发送失败
}

void emws_socket_set_flag_write(emws_socket_t* p_emws)
{
    assert(NULL != p_emws);
    p_emws->flag |= EM_SOCKET_WRITE;
}

void emws_socket_clear_flag_write(emws_socket_t* p_emws)
{
    assert(NULL != p_emws);
    p_emws->flag &= ~(uint32_t)(EM_SOCKET_WRITE);
}

//////////////////////////////////////////////////////////////////////////

static EM_BOOL cb_emws_socket_open(int eventType, const EmscriptenWebSocketOpenEvent *e, void *userData)
{
    emws_socket_t* p_emws = (emws_socket_t*)userData;
    assert(NULL != p_emws);
    p_emws->status = EMWS_SOCKET_OPEN;

    // websocket连接成功
    //LOG("emws_socket_t open ok");

    if (NULL != p_emws->cb_open)
    {
        p_emws->cb_open(p_emws, 0, 0); // 新连接成功
    }

    return EM_TRUE;
}

static EM_BOOL cb_emws_socket_close(int eventType, const EmscriptenWebSocketCloseEvent *e, void *userData)
{
    emws_socket_t* p_emws = (emws_socket_t*)userData;
    assert(NULL != p_emws);

    p_emws->status = EMWS_SOCKET_CLOSE;

    // websocket连接关闭
    //LOG("emws_socket_t close");

    if (NULL != p_emws->cb_close)
    {
        p_emws->cb_close(p_emws, 0);
    }

    return EM_TRUE;
}

static EM_BOOL cb_emws_socket_error(int eventType, const EmscriptenWebSocketErrorEvent *e, void *userData)
{
    emws_socket_t* p_emws = (emws_socket_t*)userData;
    assert(NULL != p_emws);

    bool open = (EMWS_SOCKET_OPEN == p_emws->status) ? true : false;

    p_emws->status = EMWS_SOCKET_ERROR;

    // websocket连接失败
    //LOG("emws_socket_t error");

    if (open)
    {
        if (NULL != p_emws->cb_open)
        {
            p_emws->cb_open(p_emws, 0, 1); // 新连接失败
        }
    }
    else
    {
        if (NULL != p_emws->cb_error)
        {
            p_emws->cb_error(p_emws, 0);
        }
    }

    return EM_TRUE;
}

static EM_BOOL cb_emws_socket_message(int eventType, const EmscriptenWebSocketMessageEvent *e, void *userData)
{
    emws_socket_t* p_emws = (emws_socket_t*)userData;
    assert(NULL != p_emws);

    // message
    //LOG("emws_socket_t message");
    if (NULL != p_emws->cb_recv)
    {
        p_emws->cb_recv(p_emws, 0, e);
    }

    return EM_TRUE;
}
