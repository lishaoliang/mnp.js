#include "emscripten.h"
#include <SDL.h>
#include "em_util/em_log.h"
#include "em_util/em_timer.h"
#include "ff_dec/ffmpeg_dec.h"
#include "em_gl/gl_ctx.h"
#include "main_app/main_app.h"
#include "emscripten/threading.h"
#include "emscripten/websocket.h"
#include "mem/klb_mem.h"
#include <assert.h>


extern main_app_t* g_main_app = NULL;
static void cb_main_loop(void);


static int g_next_id = 1000;

static int get_next_id()
{
    // 获取下一个ID
    int id = g_next_id;

    g_next_id++;
    if (0x7FFF0000 <= g_next_id)
    {
        g_next_id = 1000;
    }

    return id;
}

int main() 
{
    // 打印是否支持
    LOG("emscripten_websocket_is_supported: %d", emscripten_websocket_is_supported());
    LOG("emscripten_has_threading_support: %d", emscripten_has_threading_support());

    // 初始化 SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        LOG_S("Unable to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    // 初始化 ffmpeg
    ffmpeg_dec_init();

    // 初始化 opengl
    int w = 0, h = 0, is_full = 0;
    emscripten_get_canvas_size(&w, &h, &is_full);
    LOG("C get canvas size:[%d,%d,%d]", w, h, is_full);
    //int w = 960, h = 540;
    emgl_ctx_init(w, h, 32);

    // 初始化 main_app_t
    g_main_app = main_app_create();
    assert(NULL != g_main_app);

    main_app_start(g_main_app);

    emscripten_set_main_loop(cb_main_loop, 60, 0);

    LOG("C main loop start.");
    return 0;
}


static void cb_main_loop(void)
{
    // fps = 0时, 目测 100-120 帧左右
    uint32_t now = em_get_ticks();

    main_app_run(g_main_app, now);
}


//////////////////////////////////////////////////////////////////////////
// 导出函数

/// @brief 测试打印,是否加载成功
EMSCRIPTEN_KEEPALIVE int mnpjs_hello(int n)
{
    // Test
    LOG("C mnpjs_hello:[%d]", n);

    return n;
}

/// @brief 模块退出, 清理资源
EMSCRIPTEN_KEEPALIVE int mnpjs_quit(int status)
{
    // Test
    LOG("C mnpjs_quit.[%d]", status);

    // stop
    if (NULL != g_main_app)
    {
        main_app_stop(g_main_app);
    }

    // destroy
    KLB_FREE_BY(g_main_app, main_app_destroy);
    
    SDL_Quit();
    LOG("C mnpjs_quit over.");

    return 0;
}

/// @brief 控制命令: 设置参数等
EMSCRIPTEN_KEEPALIVE char* mnpjs_control(const char* p_param)
{
    return main_app_control(g_main_app, p_param);
}


/// @brief 打开某个设备
/// @param [in] *p_name  名称
/// @param [in] *p_param json格式的参数信息
/// \n 例如: {"ip"="127.0.0.1","port"=8000,"protocol"="WS-MNP","path"="/wsmnp"}
/// @return int 0.成功; 非0.错误码
EMSCRIPTEN_KEEPALIVE int mnpjs_open(const char* p_name, const char* p_param)
{
    //LOG("C mnp.mian.mnpjs_open:[%s]:[%s]", p_name, p_param);
    int id = get_next_id();

    int ret = main_app_open(g_main_app, id, p_name, p_param);

    if (0 != ret)
    {
        return 0;
    }

    return id;
}


/// @brief 关闭
/// @param [in] *p_name  名称
/// @return int 0.成功; 非0.错误码
EMSCRIPTEN_KEEPALIVE int mnpjs_close(const char* p_name)
{
    //LOG("C mnp.mian.mnpjs_close:[%s]", p_name);
    int ret = main_app_close(g_main_app, p_name);

    return 0;
}


/// @brief 发起请求
/// @param [in]  *p_name 名称
/// @param [in]  *p_req  请求的数据(json格式)
/// @param [out] **p_res 如果请求成功,输出设备回复的协议数据(json格式) 
/// @return int 0.成功; 非0.错误码
EMSCRIPTEN_KEEPALIVE int mnpjs_request(const char* p_name, const char* p_req)
{
    //LOG("C mnp.mian.mnpjs_request:[%s]:[%s]", p_name, p_req);

    int id = get_next_id();

    int ret = main_app_request(g_main_app, id, p_name, p_req);

    if (0 != ret)
    {
        return 0;
    }

    return id;
}


/// @brief 获取异步调用结果
EMSCRIPTEN_KEEPALIVE char* mnpjs_get_result(int id)
{
    return main_app_get_result(g_main_app, id);
}
