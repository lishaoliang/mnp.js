///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//  Created: 2019/05/26
//
/// @file    klb_htab.h
/// @brief   hash table查找算法
///  \n 参考: https://github.com/torvalds/linux/blob/v5.1-rc7/security/selinux/ss/hashtab.h
///  \n 参考: https://github.com/torvalds/linux/blob/v5.1-rc7/security/selinux/ss/hashtab.c
///  \n 可选优化方案:
///  \n  1. 采用比较回调函数替代(klb_htab_node_t.hash, klb_htab_node_t.p_key, klb_htab_node_t.key_len)
///  \n 以节省空间, 但会增加时间复杂度 和 提高使用难度
///  \n 此模块主要面向应用开发,优先考虑降低使用难度 
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @version 0.1
/// @history 修改历史
///  \n 2019/05/26 0.1 创建文件
/// @warning 没有警告
///////////////////////////////////////////////////////////////////////////
#ifndef __KLB_HTAB_H__
#define __KLB_HTAB_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif


/// @def   KLB_HTAB_AUTO_MALLOC
/// @brief hash table模块内部自动分配节点内存
#define KLB_HTAB_AUTO_MALLOC        0


/// @def   KLB_HTAB_AUTO_MALLOC
/// @brief hash table模块内部不要分配节点内存
#define KLB_HTAB_NOT_MALLOC        1


/// @struct klb_htab_node_t
/// @brief  hash table node节点
typedef struct klb_htab_node_t_ klb_htab_node_t;


/// @struct klb_htab_node_t
/// @brief  hash table node节点
typedef struct klb_htab_node_t_
{
    void*                   p_data;     ///< 节点数据
    klb_htab_node_t*        p_next;     ///< 下一个节点
    void*                   p_key;      ///< key

    uint32_t                hash;       ///< hash值
    uint32_t                key_len;    ///< key长度
}klb_htab_node_t;


/// @struct klb_htab_t
/// @brief  hash table查找
typedef struct klb_htab_t_ klb_htab_t;


/// @brief 创建htab对象
/// @param [in] ht_max       hash table大小
/// @param [in] auto_malloc  节点内存使用方式: KLB_HTAB_AUTO_MALLOC.内部申请; KLB_HTAB_NOT_MALLOC.调用者传入
/// @return klb_htab_t* htab对象
KLB_API klb_htab_t* klb_htab_create(uint32_t ht_max, int auto_malloc);


/// @brief 销毁htab对象
/// @param [in] *p_htab     htab对象
/// @return 无
KLB_API void klb_htab_destroy(klb_htab_t* p_htab);


/// @brief 清空htab回调函数
/// @param [in] *p_obj      传参对象
/// @param [in] *p_data     数据
/// @return int 0
typedef int(*klb_htab_clean_cb)(void* p_obj, void* p_data);


/// @brief 清空htab
/// @param [in] *p_list     htab对象
/// @param [in] cb_clean    清理回调函数
/// @param [in] *p_obj      传参对象
/// @return 无
KLB_API int klb_htab_clean(klb_htab_t* p_htab, klb_htab_clean_cb cb_clean, void* p_obj);


/// @brief 向htab对象压入数据
/// @param [in] *p_htab      htab对象
/// @param [in] *p_key       key关键字(非NULL)
/// @param [in] key_len      key长度(按char计算)
/// @param [in] *p_data      数据
/// @param [in] *p_htab_node 当KLB_HTAB_NOT_MALLOC时,传入节点指针
/// @return int 0.成功; 非0.失败
/// @note 1. htab对象并不负责数据释放
///  \n   2. 已经存在完全一致的key时, 返回失败
///  \n   3. p_key指针被缓存, 用于hash冲突时匹配
KLB_API int klb_htab_push(klb_htab_t* p_htab, void* p_key, uint32_t key_len, void* p_data, klb_htab_node_t* p_htab_node);


/// @brief 按key移除
/// @param [in] *p_list      htab对象
/// @param [in] *p_key       key关键字(非NULL)
/// @param [in] key_len      key长度
/// @return void* 数据指针 或 NULL(未找到)
KLB_API void* klb_htab_remove(klb_htab_t* p_htab, void* p_key, uint32_t key_len);


/// @brief 按key寻找值
/// @param [in] *p_list      htab对象
/// @param [in] *p_key       key关键字(非NULL)
/// @param [in] key_len      key长度
/// @return void* 数据指针 或 NULL(未找到)
KLB_API void* klb_htab_find(klb_htab_t* p_htab, const void* p_key, uint32_t key_len);


/// @brief 获取当前节点数目
/// @param [in] *p_list      htab对象
/// @return uint32_t 当前节点数
KLB_API uint32_t klb_htab_size(klb_htab_t* p_htab);


#ifdef __cplusplus
}
#endif

#endif // __KLB_HTAB_H__
//end
