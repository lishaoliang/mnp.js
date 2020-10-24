#ifndef __LIBMNPXHR_H__
#define __LIBMNPXHR_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif


/// @brief xhr打开结果 的回调函数
/// @param [in] *p_user_data    用户指针
/// @param [in] code            0.打开成功; 非0.打开失败
/// @return 0 
typedef int(*mnpxhr_open_cb)(void* p_user_data, int code);

/// @brief xhr出现错误 的回调函数
/// @param [in] *p_user_data    用户指针
/// @param [in] code            0.正常结束; 非0.异常错误码
/// @return 0 
typedef int(*mnpxhr_error_cb)(void* p_user_data, int code);

/// @brief 当收到数据的回调函数
/// @param [in] *p_user_data    用户指针
/// @param [in] *p_data         数据
/// @param [in] data_len        数据长度
/// @return 0 
typedef int(*mnpxhr_recv_cb)(void* p_user_data, const char* p_data, int data_len);


/// @brief 打开一个xhr
/// @param [in] *p_url          URL地址
/// @param [in] *p_json_param   JSON格式的参数信息
/// @param [in] *p_user_data    用户指针
/// @param [in] cb_open         open的回调函数
/// @param [in] cb_recv         收到数据的回调函数
/// @param [in] cb_error        出错或结束的回调函数
/// @return int 
///  \n return = -1   浏览器不支持xhr
///  \n return = 0    URL等错误导致无法正确xhr
///  \n 0 < return    本次xhr的ID号, 后续需要 mnpxhr_close
/// @note 仅用于处理流数据, 不适合其他类型
KLB_API int mnpxhr_open(const char* p_url, const char* p_json_param, void* p_user_data, mnpxhr_open_cb cb_open, mnpxhr_recv_cb cb_recv, mnpxhr_error_cb cb_error);


/// @brief 关闭xhr
/// @param [in] id              由mnpxhr_open产生的 大于0的ID号
/// @return 无
KLB_API void mnpxhr_close(int id);


#ifdef __cplusplus
}
#endif

#endif // __LIBMNPXHR_H__
//end
