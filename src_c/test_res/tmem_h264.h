///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2020, LGPLV3
//  Created: 2020
//
/// @file    tmem_h264.h
/// @brief   文件简要描述
/// @author  李绍良
///  \n https://github.com/lishaoliang/mnp.js
/// @version 0.1
/// @history 修改历史
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __TMEM_H264_H__
#define __TMEM_H264_H__

typedef enum tmem_h264_type_e_
{
    TMEM_H264_FRAME_B = 0,
    TMEM_H264_FRAME_I
}tmem_h264_type_e;


typedef struct tmem_h264_key_t_
{
    char*   p_sps;      ///< sps位置
    int     sps_len;    ///< sps长度

    char*   p_pps;      ///< pps位置
    int     pps_len;    ///< pps长度

    char*   p_i;        ///< i位置
    int     i_len;      ///< i长度

    int     len;        ///< 总长度
}tmem_h264_key_t;

typedef struct tmem_h264_t_ tmem_h264_t;

tmem_h264_t* tmem_h264_open(const char* p_h264, int h264_len);
void tmem_h264_close(tmem_h264_t* p_tmem_h264);

/// @brief 获取帧总数
int tmem_h264_num(tmem_h264_t* p_tmem_h264);

/// @brief 获取帧
/// @return int 返回帧长度: 0 无效帧; 大于0 有数据帧
int tmem_h264_get(tmem_h264_t* p_tmem_h264, int idx, char** p_h264, int* p_frame_type, tmem_h264_key_t* p_key);
int tmem_h264_get_next(tmem_h264_t* p_tmem_h264, char** p_h264, int* p_frame_type, tmem_h264_key_t* p_key);


#endif // __TMEM_H264_H__
//end
