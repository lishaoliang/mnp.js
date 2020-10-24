///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    em_buf_mnp.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __EM_BUF_MNP_H__
#define __EM_BUF_MNP_H__

#include "klb_type.h"
#include "em_util/em_buf.h"
#include "mnp/klb_mnp.h"

#if defined(__cplusplus)
extern "C" {
#endif


// 检查 em_buf_t 数据完整性
int em_buf_mnp_check(const em_buf_t* p_buf);


// 将 em_buf_t 帧数据提取出媒体
int em_buf_mnp_join_md(const em_buf_t* p_buf, klb_mnp_media_t* p_out_md, char** p_data, int* p_data_len);

// 将 em_buf_t 帧数据提取出媒体
em_buf_t* em_buf_mnp_join_md2(const em_buf_t* p_buf);

// 将 em_buf_t 帧数据提取出文本
int em_buf_mnp_join_txt(const em_buf_t* p_buf, klb_mnp_common_t* p_out_mnp_txt, char** p_data, int* p_data_len);

// 将txt打包成em_buf_t
em_buf_t* em_buf_mnp_pack_txt(const char* p_txt, uint32_t sequence, uint32_t uid);


#ifdef __cplusplus
}
#endif

#endif // __EM_BUF_MNP_H__
//end
