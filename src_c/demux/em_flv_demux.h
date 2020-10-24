#ifndef __EM_FLV_DEMUX_H__
#define __EM_FLV_DEMUX_H__

#include "klb_type.h"
#include "em_util/em_buf.h"
#include "list/klb_list.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct em_flv_demux_t_ em_flv_demux_t;


em_flv_demux_t* em_flv_demux_create();
void em_flv_demux_destroy(em_flv_demux_t* p_flv_demux);


/// @brief 解分离flv数据流
/// @param [in] *p_flv_demux    模块
/// @param [in] *p_data         流数据
/// @param [in] data_len        数据长度
/// @param [in,out] *p_list_out 解析出来的数据帧,放入此链表中
/// @return 0.正常; -1. 数据格式错误
int em_flv_demux_do(em_flv_demux_t* p_flv_demux, const char* p_data, int data_len, klb_list_t* p_list_out);


#ifdef __cplusplus
}
#endif

#endif // __EM_FLV_DEMUX_H__
//end
