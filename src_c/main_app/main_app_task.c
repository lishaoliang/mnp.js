#include "main_app/main_app_task.h"
#include "mem/klb_mem.h"
#include <assert.h>


main_app_task_t* main_app_task_create()
{
    main_app_task_t* p_task = KLB_MALLOC(main_app_task_t, 1, 0);
    KLB_MEMSET(p_task, 0, sizeof(main_app_task_t));

    p_task->p_item_list = klb_list_create();

    return p_task;
}

void main_app_task_destroy(main_app_task_t* p_task)
{
    assert(NULL != p_task);

    while (0 < klb_list_size(p_task->p_item_list))
    {
        main_app_task_item_t* p_item = (main_app_task_item_t*)klb_list_pop_head(p_task->p_item_list);

        KLB_FREE_BY(p_item->p_buf, em_buf_unref_next);
        KLB_FREE(p_item);
    }

    KLB_FREE_BY(p_task->p_item_list, klb_list_destroy);
    KLB_FREE(p_task);
}

int main_app_task_push_new(main_app_task_t* p_task, int id, const char* p_name)
{
    assert(NULL != p_task);

    main_app_task_item_t* p_item = KLB_MALLOC(main_app_task_item_t, 1, 0);
    KLB_MEMSET(p_item, 0, sizeof(main_app_task_item_t));

    p_item->id = id;
    strncpy(p_item->name, p_name, EM_NET_NAME_LEN);

    p_item->status = MAIN_APP_TASK_STATUS_WAITING;

    klb_list_push_tail(p_task->p_item_list, p_item);

    return 0;
}

int main_app_task_push_result(main_app_task_t* p_task, const char* p_name, em_buf_t* p_buf, int code)
{
    assert(NULL != p_task);

    klb_list_iter_t* p_iter = klb_list_begin(p_task->p_item_list);
    while (NULL != p_iter)
    {
        main_app_task_item_t* p_item = (main_app_task_item_t*)klb_list_data(p_iter);

        if (0 == strcmp(p_item->name, p_name))
        {
            p_item->code = code;
            p_item->p_buf = p_buf;
            p_item->status = MAIN_APP_TASK_STATUS_RESULT;

            return p_item->id;
        }

        p_iter = klb_list_next(p_iter);
    }

    return -1;
}

int main_app_task_pop_result(main_app_task_t* p_task, int id, em_buf_t** p_buf, int* p_code)
{
    assert(NULL != p_task);

    klb_list_iter_t* p_iter = klb_list_begin(p_task->p_item_list);
    while (NULL != p_iter)
    {
        main_app_task_item_t* p_item = (main_app_task_item_t*)klb_list_data(p_iter);

        if (id == p_item->id &&
            MAIN_APP_TASK_STATUS_RESULT == p_item->status)
        {
            *p_buf = p_item->p_buf;
            *p_code = p_item->code;

            klb_list_remove(p_task->p_item_list, p_iter);

            KLB_FREE(p_item);
            return 0;
        }

        p_iter = klb_list_next(p_iter);
    }

    return 1;
}
