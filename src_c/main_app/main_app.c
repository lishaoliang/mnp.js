#include "main_app.h"
#include "main_app_in.h"
#include "mem/klb_mem.h"
#include "test_res/tmem_h264.h"
#include "test_res/mnp_h264_res.h"
#include "em_net/em_socket_manage.h"
#include "em_net/em_conn_manage.h"
#include "ff_dec/ff_dec_manage.h"
#include "em_util/em_buf_mnp.h"
#include "em_util/em_log.h"
#include "em_gl/gl_ctx.h"
#include "em_gl/gl_yuv.h"
#include "emscripten/em_asm.h"
#include "libavcodec/avcodec.h"
#include "em_ws/em_ws.h"
#include <assert.h>


main_app_t* main_app_create()
{
    main_app_t* p_app = KLB_MALLOC(main_app_t, 1, 0);
    KLB_MEMSET(p_app, 0, sizeof(main_app_t));

    p_app->p_socket_manage = em_socket_manage_create();
    p_app->p_conn_manage = em_conn_manage_create(p_app->p_socket_manage);

    p_app->p_gl_ctx = emgl_ctx_create();
    p_app->p_gl_yuv = emgl_yuv_create();

    p_app->p_dec_manage = ff_dec_manage_create();

    p_app->p_tmem_h264 = tmem_h264_open(g_res_key_map[0].p_data, g_res_key_map[0].data_len);


    p_app->p_open_task = main_app_task_create();
    p_app->p_request_task = main_app_task_create();

    return p_app;
}

void main_app_destroy(main_app_t* p_app)
{
    assert(NULL != p_app);

    KLB_FREE_BY(p_app->p_request_task, main_app_task_destroy);
    KLB_FREE_BY(p_app->p_open_task, main_app_task_destroy);

    KLB_FREE_BY(p_app->p_tmem_h264, tmem_h264_close);
    KLB_FREE_BY(p_app->p_dec_manage, ff_dec_manage_destroy);

    KLB_FREE_BY(p_app->p_gl_yuv, emgl_yuv_destroy);
    KLB_FREE_BY(p_app->p_gl_ctx, emgl_ctx_destroy);

    KLB_FREE_BY(p_app->p_conn_manage, em_conn_manage_destroy);
    KLB_FREE_BY(p_app->p_socket_manage, em_socket_manage_destroy);
    KLB_FREE(p_app);
}

int main_app_start(main_app_t* p_app)
{
    assert(NULL != p_app);
    // 开启工作线程

    int ret = ff_dec_manage_start(p_app->p_dec_manage);

    return ret;
}

void main_app_stop(main_app_t* p_app)
{
    assert(NULL != p_app);

    ff_dec_manage_stop(p_app->p_dec_manage);
}

static void main_app_push_test_h264(main_app_t* p_app, uint32_t now_ticks)
{
    if (ABS_SUB(p_app->last_tc, now_ticks) < 33)
    {
        return;
    }

    p_app->last_tc = now_ticks;

    // TEST
    char* p_h264 = NULL;
    int frame_type = TMEM_H264_FRAME_B;
    int h264_len = tmem_h264_get_next(p_app->p_tmem_h264, &p_h264, &frame_type, NULL);
    if (0 < h264_len)
    {
        //uint8_t* p_data = KLB_MALLOC(uint8_t, h264_len, 0);
        //memcpy(p_data, p_h264, h264_len);

        //ff_dec_manage_push(p_app->p_dec_manage, AV_CODEC_ID_H264, p_data, h264_len, now_ticks, now_ticks);
    }
}

static int callback_mnpjs(int id, int status)
{
    // 回调JS脚本中的 mnpjs.callback 函数
    // 通知某个请求ID处理完成了, 可以通过 get_result 函数获取结果
    int x = EM_ASM_INT({

        if ((typeof mnpjs !== 'undefined') && (typeof mnpjs.callback == 'function')) {
            mnpjs.callback($0, $1);
        } else {
            console.log('C mnpjs.callback error! not found!');
        };

        //return $0 + $1;
        return 0;
    }, id, status);

    return 0;
}

static void main_app_text(main_app_t* p_app, uint32_t now_ticks)
{
    while (true)
    {

        int code = EM_CMC_OK;
        char protocol[EM_NET_NAME_BUF] = { 0 };
        char name[EM_NET_NAME_BUF] = { 0 };

        em_buf_t* p_buf = NULL;
        if (0 == em_conn_manage_recv(p_app->p_conn_manage, protocol, name, &p_buf, &code))
        {
            if (0 == code)
            {
                klb_mnp_txt_t mnp_txt = { 0 };
                char* p_data = NULL;
                int data_len = 0;

                if (0 == em_buf_mnp_join_txt(p_buf, &mnp_txt, &p_data, &data_len))
                {
                    LOG("main_app_text recv text: [%s]:[%s]:[%s]", protocol, name, p_data);
                    KLB_FREE(p_data);
                }
            }
            else if (EM_CMC_NEW_CONNECT == code || EM_CMC_ERR_NEW_CONNECT == code)
            {
                int id = main_app_task_push_result(p_app->p_open_task, name, NULL, code);
                if (0 < id)
                {
                    callback_mnpjs(id, code);
                }
            }
            else
            {
                LOG("main_app_text recv code: [%s]:[%s]:[%d]", protocol, name, code);
            }

            if (NULL != p_buf)
            {
                em_buf_unref_next(p_buf);
            }
        }
        else
        {
            break; // 没有数据, 才退出
        }
    }
}

static void main_app_stream(main_app_t* p_app, uint32_t now_ticks)
{
    while (true)
    {
        em_buf_t* p_buf = NULL;
        if (0 == em_conn_manage_recv_md(p_app->p_conn_manage, NULL, NULL, &p_buf))
        {
            ff_dec_manage_push(p_app->p_dec_manage, p_buf);
        }
        else
        {
            break; // 没有数据, 才退出
        }
    }
}

int main_app_run(main_app_t* p_app, uint32_t now_ticks)
{
    assert(NULL != p_app);
    // 直接使用主线程干活: 必须非柱塞方式, 且耗时不能太长, 保证主线程流畅


    // 网络
    em_socket_manage_run(p_app->p_socket_manage, now_ticks);


    // 从网络取文本协议数据
    main_app_text(p_app, now_ticks);


    // 从网络取数据, 并放入解码模块
    //main_app_push_test_h264(p_app, now_ticks);
    main_app_stream(p_app, now_ticks);


    // 运行解码过程
    ff_dec_manage_run(p_app->p_dec_manage, now_ticks);


    // 显示
    while (true)
    {
        em_yuv_frame_t* p_yuv = ff_dec_manage_get(p_app->p_dec_manage);
        if (NULL != p_yuv)
        {
            emgl_ctx_begin(p_app->p_gl_ctx);
            emgl_yuv_draw(p_app->p_gl_yuv, p_yuv);
            emgl_ctx_end(p_app->p_gl_ctx);

            ff_dec_manage_free(p_yuv);
        }
        else
        {
            break;
        }
    }

    return 0;
}
