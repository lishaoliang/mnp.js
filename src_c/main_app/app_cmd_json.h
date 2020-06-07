﻿///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    app_cmd_json.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __APP_CMD_JSON_H__
#define __APP_CMD_JSON_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define APP_CMD_STR_LEN     64
#define APP_CMD_STR_BUF     68
#define APP_CMD_PATH_LEN    1024
#define APP_CMD_PATH_BUF    1028


typedef struct app_cmd_open_t_
{
    int     port;

    char    ip[APP_CMD_STR_BUF];
    char    protocol[APP_CMD_STR_BUF];
    char    path[APP_CMD_PATH_BUF];

    //char    username[APP_CMD_STR_BUF];
    //char    passwd[APP_CMD_STR_BUF];
}app_cmd_open_t;


int app_cmd_parse_open(app_cmd_open_t* p_op, const char* p_param);


typedef struct app_cmd_control_t_
{
    int a;
}app_cmd_control_t;

int app_cmd_parse_control(app_cmd_control_t* p_control,const char* p_param);


#ifdef __cplusplus
}
#endif

#endif // __APP_CMD_JSON_H__
//end
