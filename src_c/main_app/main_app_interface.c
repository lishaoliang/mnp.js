#include "main_app.h"
#include "main_app_in.h"
#include "app_cmd_json.h"
#include "mem/klb_mem.h"
#include <stdio.h>
#include <assert.h>

// main app 对JS接口部分函数

char* main_app_control(main_app_t* p_app, const char* p_param)
{
    return NULL;
}

int main_app_open(main_app_t* p_app, int id, const char* p_name, const char* p_param)
{
    app_cmd_open_t op = { 0 };

    if (0 == app_cmd_parse_open(&op, p_param))
    {
        int ret = em_conn_manage_connect(p_app->p_conn_manage, op.protocol, p_name, op.ip, op.port, op.path);

        if (0 == ret)
        {
            main_app_task_push_new(p_app->p_open_task, id, p_name);
        }

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
