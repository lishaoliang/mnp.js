///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    main_app.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __MAIN_APP_H__
#define __MAIN_APP_H__


#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct main_app_t_ main_app_t;


main_app_t* main_app_create();
void main_app_destroy(main_app_t* p_app);


int main_app_start(main_app_t* p_app);
void main_app_stop(main_app_t* p_app);


int main_app_run(main_app_t* p_app, int64_t now_ticks);


char* main_app_control(main_app_t* p_app, const char* p_cmd, const char* p_lparam, const char* p_wparam);
int main_app_open(main_app_t* p_app, int id, const char* p_name, const char* p_param);
int main_app_close(main_app_t* p_app, const char* p_name);
int main_app_request(main_app_t* p_app, int id, const char* p_name, const char* p_req);
char* main_app_get_result(main_app_t* p_app, int id);


#ifdef __cplusplus
}
#endif

#endif // __MAIN_APP_H__
//end
