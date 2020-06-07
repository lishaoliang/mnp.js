///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    em_conn_manage_in.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __EM_CONN_MANAGE_IN_H__
#define __EM_CONN_MANAGE_IN_H__


#include "klb_type.h"
#include "em_net/em_conn_manage.h"
#include "em_net/em_socket_manage.h"
#include "hash/klb_hlist.h"
#include "list/klb_list.h"
#include "em_util/em_buf.h"

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct em_conn_manage_packet_txt_t_
{
    int             code;
    em_buf_t*       p_buf;

    char            protocol[EM_NET_NAME_BUF];
    char            name[EM_NET_NAME_BUF];
}em_conn_manage_packet_txt_t;


typedef struct em_conn_manage_packet_md_t_
{
    em_buf_t*       p_buf;

    char            protocol[EM_NET_NAME_BUF];
    char            name[EM_NET_NAME_BUF];
}em_conn_manage_packet_md_t;


typedef struct em_conn_manage_t_
{
    em_socket_manage_t*     p_socket_manage;
    klb_hlist_t*            p_conn_hlist;

    klb_list_t*             p_read_txt_list;
    klb_list_t*             p_read_md_list;
}em_conn_manage_t;


int em_conn_manage_push(em_conn_manage_t* p_manage, const char* p_protocol, const char* p_name, int code, em_buf_t* p_buf);


int em_conn_manage_push_md(em_conn_manage_t* p_manage, const char* p_protocol, const char* p_name, em_buf_t* p_buf);


#ifdef __cplusplus
}
#endif

#endif // __EM_CONN_MANAGE_IN_H__
//end
