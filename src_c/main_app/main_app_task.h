///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    main_app_task.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __MAIN_APP_TASK_H__
#define __MAIN_APP_TASK_H__

#include "klb_type.h"
#include "em_net/em_conn_manage.h"
#include "list/klb_list.h"

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct main_app_task_item_t_
{
    int             id;
    char            name[EM_NET_NAME_BUF];

    int             code;
    em_buf_t*       p_buf;

    int             status;
#define MAIN_APP_TASK_STATUS_WAITING    0
#define MAIN_APP_TASK_STATUS_RESULT     1
}main_app_task_item_t;


typedef struct main_app_task_t
{
    klb_list_t*     p_item_list;
}main_app_task_t;


main_app_task_t* main_app_task_create();
void main_app_task_destroy(main_app_task_t* p_task);

int main_app_task_push_new(main_app_task_t* p_task, int id, const char* p_name);
int main_app_task_push_result(main_app_task_t* p_task, const char* p_name, em_buf_t* p_buf, int code);

int main_app_task_pop_result(main_app_task_t* p_task, int id, em_buf_t** p_buf, int* p_code);


#ifdef __cplusplus
}
#endif

#endif // __MAIN_APP_TASK_H__
//end
