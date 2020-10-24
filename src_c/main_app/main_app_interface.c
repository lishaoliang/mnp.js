#include "main_app.h"
#include "main_app_in.h"
#include "app_cmd_json.h"
#include "mem/klb_mem.h"
#include <stdio.h>
#include <assert.h>

// main app 对JS接口部分函数

char* main_app_control(main_app_t* p_app, const char* p_cmd, const char* p_lparam, const char* p_wparam)
{
    if (NULL == p_cmd)
    {
        return NULL;
    }

    if (0 == strcmp("set_delay", p_cmd))
    {
        if (0 == strcmp("lowest", p_lparam)){ ff_dec_single_set_delay(p_app->p_dec_manage, FF_RATE_CTRL_LOWEST); }
        else if (0 == strcmp("lower", p_lparam)) { ff_dec_single_set_delay(p_app->p_dec_manage, FF_RATE_CTRL_LOWER); }
        else if (0 == strcmp("low", p_lparam)) { ff_dec_single_set_delay(p_app->p_dec_manage, FF_RATE_CTRL_LOW); }
        else if (0 == strcmp("middle", p_lparam)) { ff_dec_single_set_delay(p_app->p_dec_manage, FF_RATE_CTRL_MIDDLE); }
        else if (0 == strcmp("high", p_lparam)) { ff_dec_single_set_delay(p_app->p_dec_manage, FF_RATE_CTRL_HIGH); }
        else if (0 == strcmp("higher", p_lparam)) { ff_dec_single_set_delay(p_app->p_dec_manage, FF_RATE_CTRL_HIGHER); }
    }

    return NULL;
}

int main_app_open(main_app_t* p_app, int id, const char* p_name, const char* p_param)
{
    app_cmd_open_t open = { 0 };

    if (0 == app_cmd_parse_open(&open, p_param))
    {
        int ret = em_conn_manage_connect(p_app->p_conn_manage, open.protocol, p_name, open.p_url);

        if (0 == ret)
        {
            main_app_task_push_new(p_app->p_open_task, id, p_name);
        }

        app_cmd_parse_free_open(&open);
        return ret;
    }

    return 1;
}

int main_app_close(main_app_t* p_app, const char* p_name)
{
    em_conn_manage_close(p_app->p_conn_manage, p_name);

    return 0;
}

int main_app_request(main_app_t* p_app, int id, const char* p_name, const char* p_req)
{
    return 0;
}

char* main_app_get_result(main_app_t* p_app, int id)
{
    int code = 0;
    em_buf_t* p_buf = NULL;

    if (0 == main_app_task_pop_result(p_app->p_open_task, id, &p_buf, &code))
    {
        char* p_str = KLB_MALLOC(char, 36, 0);
        snprintf(p_str, 32, "{\"code\":%d}", (EM_CMC_NEW_CONNECT == code) ? 0 : code);

        KLB_FREE_BY(p_buf, em_buf_unref_next);
        return p_str;
    }

    return NULL;
}
