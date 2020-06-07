///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//  Created: 2019/05/25
//
/// @file    klb_list.h
/// @brief   精简双向链表: 非侵入式
///  \n 精简list: 仅支持压入, 弹出, 迭代访问
///  \n 参考: https://github.com/IIJ-NetBSD/netbsd-src/blob/master/external/bsd/libbind/dist/include/isc/list.h
///  \n 参考: std::list
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @version 0.1
/// @history 修改历史
///  \n 2019/05/25 0.1 创建文件
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __KLB_LIST_H__
#define __KLB_LIST_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif


/// @struct klb_list_iter_t
/// @brief  list节点 OR 迭代子
typedef struct klb_list_iter_t_ klb_list_iter_t;


/// @struct klb_list_t
/// @brief  list对象
typedef struct klb_list_t_ klb_list_t;


/// @brief 创建list对象
/// @return klb_list_t*     list对象
KLB_API klb_list_t* klb_list_create();


/// @brief 销毁list
/// @param [in] *p_list     list对象
/// @return 无
/// @note 销毁前必须自行清空链表, 否则内存泄露
KLB_API void klb_list_destroy(klb_list_t* p_list);


/// @brief 清空列表回调函数
/// @param [in] *p_obj      传参对象
/// @param [in] *p_data     数据
/// @return int 0
typedef int(*klb_list_clean_cb)(void* p_obj, void* p_data);


/// @brief 清空列表
/// @param [in] *p_list     list对象
/// @param [in] cb_clean    清理回调函数
/// @param [in] *p_obj      传参对象
/// @return 无
/// @note 方向: 从头到尾
KLB_API void klb_list_clean(klb_list_t* p_list, klb_list_clean_cb cb_clean, void* p_obj);


/// @brief 在最前面, 向list压入数据
/// @param [in] *p_list     list对象
/// @param [in] *p_data     数据
/// @return 无
/// @note list并不负责数据释放
KLB_API void klb_list_push_head(klb_list_t* p_list, void* p_data);


/// @brief 在最后面, 向list压入数据
/// @param [in] *p_list     list对象
/// @param [in] *p_data     数据
/// @return 无
/// @note list并不负责数据释放
KLB_API void klb_list_push_tail(klb_list_t* p_list, void* p_data);


/// @brief 在最前面, 弹出数据
/// @param [in] *p_list     list对象
/// @return void* 数据指针
KLB_API void* klb_list_pop_head(klb_list_t* p_list);


/// @brief 在最后面, 弹出数据
/// @param [in] *p_list     list对象
/// @return void* 数据指针
KLB_API void* klb_list_pop_tail(klb_list_t* p_list);


/// @brief 访问最前面的数据
/// @param [in] *p_list     list对象
/// @return void* 数据指针
KLB_API void* klb_list_head(klb_list_t* p_list);


/// @brief 访问最后面的数据
/// @param [in] *p_list     list对象
/// @return void* 数据指针
KLB_API void* klb_list_tail(klb_list_t* p_list);


/// @brief 当前节点数目
/// @param [in] *p_list     list对象
/// @return uint32_t 返回当前节点数目
KLB_API uint32_t klb_list_size(klb_list_t* p_list);


/// @brief 按迭代子移除节点
/// @param [in] *p_list     list对象
/// @param [in] *p_iter     iter迭代子
/// @return void* 数据指针
/// @note 1. 数据由调用者维护其生命周期
///  \n   2. 函数调用后, iter迭代子失效(内存被释放)
KLB_API void* klb_list_remove(klb_list_t* p_list, klb_list_iter_t* p_iter);


/// @brief 获取list iter对应的数据
/// @param [in] *p_iter     iter迭代子
/// @return void* 返回数据
KLB_API void* klb_list_data(klb_list_iter_t* p_iter);


/// @brief 获取最前面的迭代子
/// @param [in] *p_list      list对象
/// @return klb_list_iter_t* iter迭代子
/// @note 返回NULL, 表示无数据
KLB_API klb_list_iter_t* klb_list_begin(klb_list_t* p_list);


/// @brief 下一个迭代子
/// @param [in] *p_list      list对象
/// @return klb_list_iter_t* iter迭代子
/// @note 和klb_list_begin配合使用
KLB_API klb_list_iter_t* klb_list_next(klb_list_iter_t* p_iter);


/// @brief 获取最后面的迭代子
/// @param [in] *p_list      list对象
/// @return klb_list_iter_t* iter迭代子
/// @note 返回NULL, 表示无数据
KLB_API klb_list_iter_t* klb_list_end(klb_list_t* p_list);


/// @brief 前一个迭代子
/// @param [in] *p_list      list对象
/// @return klb_list_iter_t* iter迭代子
/// @note 和klb_list_end配合使用
KLB_API klb_list_iter_t* klb_list_prev(klb_list_iter_t* p_iter);


#ifdef __cplusplus
}
#endif

#endif // __KLB_LIST_H__
//end
