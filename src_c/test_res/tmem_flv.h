#ifndef __TMEM_FLV_H__
#define __TMEM_FLV_H__

#include "klb_type.h"
#include "em_buf.h"


typedef struct tmem_flv_t_ tmem_flv_t;


tmem_flv_t* tmem_flv_open(const char* p_data, int data_len);
void tmem_flv_close(tmem_flv_t* p_tmem_flv);


/// @brief 获取帧总数
int tmem_flv_num(tmem_flv_t* p_tmem_flv);


em_buf_t* tmem_flv_get_next(tmem_flv_t* p_tmem_flv);


#endif // __TMEM_FLV_H__
//end
