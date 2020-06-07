///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//
/// @file    klb_htab.c
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @brief   hash table查找算法
///////////////////////////////////////////////////////////////////////////
#include "hash/klb_htab.h"
#include "mem/klb_mem.h"
#include "hash/klb_hash.h"
#include <assert.h>


/// @def   KLB_HTAB_DOUBLE_HASH
/// @brief 是否采用双hash降低冲突概率
#define KLB_HTAB_DOUBLE_HASH                 1


/// @struct klb_htab_t
/// @brief  hash table查找
typedef struct klb_htab_t_
{
    klb_htab_node_t**       p_idx;      ///< hash table节点数组

    int                     auto_malloc;///< KLB_HTAB_AUTO_MALLOC.自动分配节点内存

    uint32_t                size;       ///< 当前节点数数
    uint32_t                idx_max;    ///< hash table 序号最大数
}klb_htab_t;


klb_htab_t* klb_htab_create(uint32_t ht_max, int auto_malloc)
{
    assert(0 < ht_max);

    klb_htab_t* p_htab = KLB_MALLOC(klb_htab_t, 1, 0);
    KLB_MEMSET(p_htab, 0, sizeof(klb_htab_t));

    p_htab->auto_malloc = auto_malloc;
    p_htab->idx_max = ht_max;

    // 常驻内存量: sizeof(void*) * ht_max
    p_htab->p_idx = KLB_MALLOC(klb_htab_node_t*, p_htab->idx_max, 0);
    KLB_MEMSET(p_htab->p_idx, 0, sizeof(klb_htab_node_t*) * p_htab->idx_max);

    return p_htab;
}

void klb_htab_destroy(klb_htab_t* p_htab)
{
    assert(NULL != p_htab);
    assert(0 == p_htab->size);

    KLB_FREE(p_htab->p_idx);
    KLB_FREE(p_htab);
}

int klb_htab_clean(klb_htab_t* p_htab, klb_htab_clean_cb cb_clean, void* p_obj)
{
    assert(NULL != p_htab);

    for (uint32_t i = 0; i < p_htab->idx_max; i++)
    {
        klb_htab_node_t* p_cur = p_htab->p_idx[i];

        while (p_cur)
        {
            klb_htab_node_t* p_tmp = p_cur;
            p_cur = p_cur->p_next;

            if (NULL != cb_clean)
            {
                cb_clean(p_obj, p_tmp->p_data);
            }

            if (KLB_HTAB_AUTO_MALLOC == p_htab->auto_malloc)
            {
                KLB_FREE(p_tmp);
            }
        }

        p_htab->p_idx[i] = NULL;
    }

    p_htab->size = 0;

    return 0;
}

static klb_htab_node_t* klb_htab_create_node(void* p_key, uint32_t key_len, uint32_t hash, void* p_data, klb_htab_node_t* p_next)
{
    klb_htab_node_t* p_node = KLB_MALLOC(klb_htab_node_t, 1, 0);
    KLB_MEMSET(p_node, 0, sizeof(klb_htab_node_t));

    p_node->p_key = p_key;
    p_node->key_len = key_len;
    p_node->hash = hash;
    p_node->p_data = p_data;
    p_node->p_next = p_next;

    return p_node;
}

static void klb_htab_init_node(klb_htab_node_t* p_node, void* p_key, uint32_t key_len, uint32_t hash, void* p_data, klb_htab_node_t* p_next)
{
    p_node->p_key = p_key;
    p_node->key_len = key_len;
    p_node->hash = hash;
    p_node->p_data = p_data;
    p_node->p_next = p_next;
}

int klb_htab_push(klb_htab_t* p_htab, void* p_key, uint32_t key_len, void* p_data, klb_htab_node_t* p_htab_node)
{
    assert(NULL != p_htab);
    assert(NULL != p_key);
    assert(NULL != p_data);

    uint32_t dx = klb_hash32((const char*)p_key, key_len);
    uint32_t idx = dx % p_htab->idx_max;

#if KLB_HTAB_DOUBLE_HASH
    uint32_t hash = klb_hash_hgsmi((const char*)p_key, key_len);
#else
    uint32_t hash = dx;
#endif

    klb_htab_node_t* p_cur = p_htab->p_idx[idx];
    klb_htab_node_t* p_prev = NULL;

    while (p_cur)
    {
        if (hash == p_cur->hash)
        {
            if (key_len < p_cur->key_len)
            {
                break; // 让具有相同 hash值, 但key长度小的排前面
            }
            else if (key_len == p_cur->key_len)
            {
                // hash值相等时, key长度相等, 再比较原始key
                int cmp = memcmp(p_key, p_cur->p_key, key_len);

                if (cmp < 0)
                {
                    break; // 小值排前面
                }
                else if (cmp == 0)
                {
                    return 1; //已存在值, 且完全一致的key
                }
            }
        }
        else if (hash < p_cur->hash)
        {
            break; // 让结果从小到大排列
        }

        p_prev = p_cur;
        p_cur = p_cur->p_next;
    }

    if (NULL != p_prev)
    {
        // 首位已存在值
        if (KLB_HTAB_AUTO_MALLOC == p_htab->auto_malloc)
        {
            assert(NULL == p_htab_node);
            p_prev->p_next = klb_htab_create_node(p_key, key_len, hash, p_data, p_prev->p_next);
        }
        else
        {
            assert(NULL != p_htab_node);
            klb_htab_init_node(p_htab_node, p_key, key_len, hash, p_data, p_prev->p_next);
            p_prev->p_next = p_htab_node;
        }
    }
    else
    {
        // 占第一个坑位
        if (KLB_HTAB_AUTO_MALLOC == p_htab->auto_malloc)
        {
            assert(NULL == p_htab_node);
            p_htab->p_idx[idx] = klb_htab_create_node(p_key, key_len, hash, p_data, p_htab->p_idx[idx]);
        }
        else
        {
            assert(NULL != p_htab_node);
            klb_htab_init_node(p_htab_node, p_key, key_len, hash, p_data, p_htab->p_idx[idx]);
            p_htab->p_idx[idx] = p_htab_node;
        }
    }

    p_htab->size += 1;
    return 0;
}

void* klb_htab_remove(klb_htab_t* p_htab, void* p_key, uint32_t key_len)
{
    assert(NULL != p_htab);
    assert(NULL != p_key);

    uint32_t dx = klb_hash32((const char*)p_key, key_len);
    uint32_t idx = dx % p_htab->idx_max;

#if KLB_HTAB_DOUBLE_HASH
    uint32_t hash = klb_hash_hgsmi((const char*)p_key, key_len);
#else
    uint32_t hash = dx;
#endif

    klb_htab_node_t* p_cur = p_htab->p_idx[idx];
    klb_htab_node_t* p_prev = NULL;

    while (p_cur)
    {
        if (hash == p_cur->hash)
        {
            if (key_len == p_cur->key_len &&
                0 == memcmp(p_key, p_cur->p_key, key_len))
            {
                void* p_data = p_cur->p_data;

                if (NULL != p_prev)
                {
                    p_prev->p_next = p_cur->p_next;
                }
                else
                {
                    // 需要删除的在第一个坑位
                    p_htab->p_idx[idx] = p_cur->p_next;
                }

                if (KLB_HTAB_AUTO_MALLOC == p_htab->auto_malloc)
                {
                    KLB_FREE(p_cur);
                }

                p_htab->size -= 1;
                return p_data;
            }
        }
        else if (hash < p_cur->hash)
        {
            break;
        }

        p_prev = p_cur;
        p_cur = p_cur->p_next;
    }

    return NULL;
}

void* klb_htab_find(klb_htab_t* p_htab, const void* p_key, uint32_t key_len)
{
    assert(NULL != p_htab);
    assert(NULL != p_key);

    uint32_t dx = klb_hash32((const char*)p_key, key_len);
    uint32_t idx = dx % p_htab->idx_max;

#if KLB_HTAB_DOUBLE_HASH
    uint32_t hash = klb_hash_hgsmi((const char*)p_key, key_len);
#else
    uint32_t hash = dx;
#endif

    klb_htab_node_t* p_cur = p_htab->p_idx[idx];

    while (p_cur)
    {
        if (hash == p_cur->hash)
        {
            if (key_len == p_cur->key_len &&
                0 == memcmp(p_key, p_cur->p_key, key_len))
            {
                return p_cur->p_data;
            }
        }
        else if (hash < p_cur->hash)
        {
            break;
        }

        p_cur = p_cur->p_next;
    }

    return NULL;
}

uint32_t klb_htab_size(klb_htab_t* p_htab)
{
    assert(NULL != p_htab);

    return p_htab->size;
}
