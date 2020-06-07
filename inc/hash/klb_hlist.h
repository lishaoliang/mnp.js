///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//  Created: 2019/06/01
//
/// @file    klb_hlist.h
/// @brief   hash table 与 list组合使用
///  \n 弥补list不适合查找的应用场景
///  \n 适用场景: 按k:v格式, 快速查找, 删除数据, 大中型场景
///  \n 每个节点需额外 sizeof(klb_hlist_iter_t) + key_len 的内存
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @version 0.1
/// @history 修改历史
///  \n 2019/06/01 0.1 创建文件
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __KLB_HLIST_H__
#define __KLB_HLIST_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif


/// @struct klb_hlist_iter_t
/// @brief  hlist节点 OR 迭代子
typedef struct klb_hlist_iter_t_ klb_hlist_iter_t;


/// @struct klb_hlist_t
/// @brief  hlist对象
typedef struct klb_hlist_t_ klb_hlist_t;


/// @brief 创建hlist对象
/// @param [in] ht_max       hash table大小
/// @return klb_hlist_t*     hlist对象
KLB_API klb_hlist_t* klb_hlist_create(uint32_t ht_max);


/// @brief 销毁hlist对象
/// @param [in] *p_list     hlist对象
/// @return 无
/// @note 销毁前必须自行清空, 否则内存泄露
KLB_API void klb_hlist_destroy(klb_hlist_t* p_list);


/// @brief 清空列表回调函数
/// @param [in] *p_obj      传参对象
/// @param [in] *p_data     数据
/// @return int 0
typedef int(*klb_hlist_clean_cb)(void* p_obj, void* p_data);


/// @brief 清空列表
/// @param [in] *p_list     hlist对象
/// @param [in] cb_clean    清理回调函数
/// @param [in] *p_obj      传参对象
/// @return 无
/// @note 方向: 从头到尾
KLB_API void klb_hlist_clean(klb_hlist_t* p_list, klb_hlist_clean_cb cb_clean, void* p_obj);


/// @brief 在最前面, 向hlist压入数据
/// @param [in] *p_list     hlist对象
/// @param [in] *p_key      key关键字(非NULL)
/// @param [in] key_len     key长度(按char计算)
/// @param [in] *p_data     数据
/// @return int 0.成功; 非0.失败
/// @note 1. hlist并不负责数据释放
///  \n   2. hlist采用已解决hash冲突的查找算法, 不存在hash值冲突问题
///  \n   3. 因hlist带有链表特性, 对已存在完全一致的key时, 返回失败, 由调用者决定对如何处理完全一致的key
KLB_API int klb_hlist_push_head(klb_hlist_t* p_list, const void* p_key, uint32_t key_len, void* p_data);


/// @brief 在最后面, 向hlist压入数据
/// @param [in] *p_list     hlist对象
/// @param [in] *p_key      key关键字(非NULL)
/// @param [in] key_len     key长度(按char计算)
/// @param [in] *p_data     数据
/// @return int 0.成功; 非0.失败
/// @note 1. hlist并不负责数据释放
///  \n   2. hlist采用已解决hash冲突的查找算法, 不存在hash值冲突问题
///  \n   3. 因hlist带有链表特性, 对已存在完全一致的key时, 返回失败, 由调用者决定对如何处理完全一致的key
KLB_API int klb_hlist_push_tail(klb_hlist_t* p_list, const void* p_key, uint32_t key_len, void* p_data);


/// @brief 在最前面, 弹出数据
/// @param [in] *p_list     hlist对象
/// @return void* 数据指针
KLB_API void* klb_hlist_pop_head(klb_hlist_t* p_list);


/// @brief 在最后面, 弹出数据
/// @param [in] *p_list     hlist对象
/// @return void* 数据指针
KLB_API void* klb_hlist_pop_tail(klb_hlist_t* p_list);


/// @brief 访问最前面的数据
/// @param [in] *p_list     hlist对象
/// @return void* 数据指针
KLB_API void* klb_hlist_head(klb_hlist_t* p_list);


/// @brief 访问最后面的数据
/// @param [in] *p_list     hlist对象
/// @return void* 数据指针
KLB_API void* klb_hlist_tail(klb_hlist_t* p_list);


/// @brief 当前节点数目
/// @param [in] *p_list    hlist对象
/// @return uint32_t 返回节点数目
KLB_API uint32_t klb_hlist_size(klb_hlist_t* p_list);


/// @brief 按迭代子移除节点
/// @param [in] *p_list     hlist对象
/// @param [in] *p_iter     iter迭代子
/// @return void* 数据指针
/// @note 1. 数据由调用者维护其生命周期
///  \n   2. 函数调用后, iter迭代子失效(内存被释放)
KLB_API void* klb_hlist_remove(klb_hlist_t* p_list, klb_hlist_iter_t* p_iter);


/// @brief 获取list iter对应的数据
/// @param [in] *p_iter     iter迭代子
/// @return void* 返回数据
KLB_API void* klb_hlist_data(klb_hlist_iter_t* p_iter);


/// @brief 获取最前面的迭代子
/// @param [in] *p_list      hlist对象
/// @return klb_list_iter_t* iter迭代子
/// @note 返回NULL, 表示无数据
KLB_API klb_hlist_iter_t* klb_hlist_begin(klb_hlist_t* p_list);


/// @brief 下一个迭代子
/// @param [in] *p_iter      iter迭代子
/// @return klb_list_iter_t* 下一个iter迭代子
/// @note 和klb_hlist_begin配合使用
KLB_API klb_hlist_iter_t* klb_hlist_next(klb_hlist_iter_t* p_iter);


/// @brief 获取最后面的迭代子
/// @param [in] *p_list      hlist对象
/// @return klb_list_iter_t* iter迭代子
/// @note 返回NULL, 表示无数据
KLB_API klb_hlist_iter_t* klb_hlist_end(klb_hlist_t* p_list);


/// @brief 前一个迭代子
/// @param [in] *p_iter      iter迭代子
/// @return klb_list_iter_t* 前一个iter迭代子
/// @note 和klb_hlist_end配合使用
KLB_API klb_hlist_iter_t* klb_hlist_prev(klb_hlist_iter_t* p_iter);


/// @brief 获取迭代子的key指针
/// @param [in]  *p_iter        iter迭代子
/// @param [out] *p_key_len     key长度
/// @return void* key指针
KLB_API void* klb_hlist_key(klb_hlist_iter_t* p_iter, uint32_t* p_key_len);


/// @brief 更新数据(只能更新已经存在的key)
/// @param [in] *p_list     hlist对象
/// @param [in] *p_key      key关键字(非NULL)
/// @param [in] key_len     key长度(按char计算)
/// @param [in] *p_data     数据
/// @return void*   NULL.失败,未找到原数据; 非NULL.成功,被更新的数据
/// @note 1. 只能更新已经存在的key
KLB_API void* klb_hlist_update(klb_hlist_t* p_list, const void* p_key, uint32_t key_len, void* p_data);



/// @brief 按key寻找迭代子
/// @param [in] *p_list      hlist对象
/// @param [in] *p_key       key关键字(非NULL)
/// @param [in] key_len      key长度
/// @return klb_hlist_iter_t* 迭代子指针 或 NULL(未找到)
KLB_API klb_hlist_iter_t* klb_hlist_find_iter(klb_hlist_t* p_list, const void* p_key, uint32_t key_len);


/// @brief 按key寻找值
/// @param [in] *p_list      hlist对象
/// @param [in] *p_key       key关键字(非NULL)
/// @param [in] key_len      key长度
/// @return void* 数据指针 或 NULL(未找到)
KLB_API void* klb_hlist_find(klb_hlist_t* p_list, const void* p_key, uint32_t key_len);


/// @brief 按key移除节点
/// @param [in] *p_list      hlist对象
/// @param [in] *p_key       key关键字(非NULL)
/// @param [in] key_len      key长度
/// @return void* 数据指针 或 NULL(未找到)
KLB_API void* klb_hlist_remove_bykey(klb_hlist_t* p_list, const void* p_key, uint32_t key_len);


/// @brief 对节点进行排序
/// @param [in] *p_list      hlist对象
/// @return 无
KLB_API void klb_hlist_qsort(klb_hlist_t* p_list);


#ifdef __cplusplus
}
#endif

#endif // __KLB_HLIST_H__
//end
