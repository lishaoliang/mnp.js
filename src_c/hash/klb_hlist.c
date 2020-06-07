///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//
/// @file    klb_hlist.c
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @brief   hash table 与 list组合使用
///////////////////////////////////////////////////////////////////////////
#include "hash/klb_hlist.h"
#include "hash/klb_htab.h"
#include "mem/klb_mem.h"
#include "log/klb_log.h"
#include <assert.h>


/// @struct klb_hlist_iter_t
/// @brief  hlist节点 OR 迭代子
typedef struct klb_hlist_iter_t_
{
    void*               p_data;        ///< 节点数据

    klb_hlist_iter_t*   p_prev;        ///< 前一个节点
    klb_hlist_iter_t*   p_next;        ///< 后一个节点

    klb_htab_node_t     htab_node;     ///< hash table节点

    uint32_t            key_len;       ///< 节点key的长度
    char                key[1];        ///< 可变长key
}klb_hlist_iter_t;


/// @struct klb_hlist_t
/// @brief  hlist对象
typedef struct klb_hlist_t_
{
    klb_hlist_iter_t*   p_head;         ///< 起始节点
    klb_hlist_iter_t*   p_tail;         ///< 末尾节点

    klb_htab_t*         p_htab;         ///< hash table

    uint32_t            size;           ///< 节点成员数目
}klb_hlist_t;

klb_hlist_t* klb_hlist_create(uint32_t ht_max)
{
    klb_hlist_t* p_list = KLB_MALLOC(klb_hlist_t, 1, 0);
    KLB_MEMSET(p_list, 0, sizeof(klb_hlist_t));

    p_list->p_htab = klb_htab_create(ht_max, KLB_HTAB_NOT_MALLOC);

    return p_list;
}

void klb_hlist_destroy(klb_hlist_t* p_list)
{
    assert(NULL != p_list);
    assert(0 == p_list->size);
    assert(NULL == p_list->p_head);
    assert(NULL == p_list->p_tail);

    // 销毁前必须释放所有节点

    KLB_FREE_BY(p_list->p_htab, klb_htab_destroy);
    KLB_FREE(p_list);
}


void klb_hlist_clean(klb_hlist_t* p_list, klb_hlist_clean_cb cb_clean, void* p_obj)
{
    assert(NULL != p_list);

    while (true)
    {
        void* p_data = klb_hlist_pop_head(p_list);

        if (NULL != p_data)
        {
            if (NULL != cb_clean)
            {
                cb_clean(p_obj, p_data);
            }
        }
        else
        {
            break;
        }
    }
}

int klb_hlist_push_head(klb_hlist_t* p_list, const void* p_key, uint32_t key_len, void* p_data)
{
    assert(NULL != p_list);
    assert(NULL != p_key);

    if (NULL != p_key && NULL != p_data)
    {
        klb_hlist_iter_t* p_iter = KLB_MALLOC(klb_hlist_iter_t, 1, key_len);

        memcpy(p_iter->key, p_key, key_len);
        p_iter->key_len = key_len;

        if (0 != klb_htab_push(p_list->p_htab, p_iter->key, p_iter->key_len, p_iter, &p_iter->htab_node))
        {
            // 放入失败, 已经存在相同的 key
            KLB_FREE(p_iter);
            return 1;
        }

        p_iter->p_data = p_data;

        p_iter->p_prev = NULL;
        p_iter->p_next = p_list->p_head;

        // 设置后节点的 p_prev指向
        if (NULL != p_iter->p_next)
        {
            p_iter->p_next->p_prev = p_iter;
        }

        // 如果插入首个元素
        if (NULL == p_list->p_tail)
        {
            assert(0 == p_list->size);
            p_list->p_tail = p_iter;
        }

        p_list->p_head = p_iter;
        p_list->size += 1;

        return 0;
    }

    return 1;
}

int klb_hlist_push_tail(klb_hlist_t* p_list, const void* p_key, uint32_t key_len, void* p_data)
{
    assert(NULL != p_list);
    assert(NULL != p_key);

    if (NULL != p_key && NULL != p_data)
    {
        klb_hlist_iter_t* p_iter = KLB_MALLOC(klb_hlist_iter_t, 1, key_len);

        memcpy(p_iter->key, p_key, key_len);
        p_iter->key_len = key_len;

        if (0 != klb_htab_push(p_list->p_htab, p_iter->key, p_iter->key_len, p_iter, &p_iter->htab_node))
        {
            // 放入失败, 已经存在相同的 key
            KLB_FREE(p_iter);
            return 1;
        }

        p_iter->p_data = p_data;

        p_iter->p_prev = p_list->p_tail;
        p_iter->p_next = NULL;

        // 设置前节点的 p_next
        if (NULL != p_iter->p_prev)
        {
            p_iter->p_prev->p_next = p_iter;
        }

        // 如果插入首个元素
        if (NULL == p_list->p_head)
        {
            assert(0 == p_list->size);
            p_list->p_head = p_iter;
        }

        p_list->p_tail = p_iter;
        p_list->size += 1;

        return 0;
    }

    return 1;
}

void* klb_hlist_pop_head(klb_hlist_t* p_list)
{
    assert(NULL != p_list);

    klb_hlist_iter_t* p_iter = p_list->p_head;

    if (NULL != p_iter)
    {
        assert(0 < p_list->size);
        p_list->p_head = p_iter->p_next;
        p_list->size -= 1;

        if (NULL != p_iter->p_next)
        {
            p_iter->p_next->p_prev = NULL;
            assert(0 < p_list->size);
        }
        else
        {
            p_list->p_tail = NULL;
            assert(0 == p_list->size);
        }

        void* p_data = p_iter->p_data;
        void* p_remove = klb_htab_remove(p_list->p_htab, p_iter->key, p_iter->key_len);
        assert(p_remove == p_iter);

        KLB_FREE(p_iter);
        return p_data;
    }

    return NULL;
}

void* klb_hlist_pop_tail(klb_hlist_t* p_list)
{
    assert(NULL != p_list);

    klb_hlist_iter_t* p_iter = p_list->p_tail;

    if (NULL != p_iter)
    {
        assert(0 < p_list->size);
        p_list->p_tail = p_iter->p_prev;
        p_list->size -= 1;

        if (NULL != p_iter->p_prev)
        {
            p_iter->p_prev->p_next = NULL;
            assert(0 < p_list->size);
        }
        else
        {
            p_list->p_head = NULL;
            assert(0 == p_list->size);
        }

        void* p_data = p_iter->p_data;
        void* p_remove = klb_htab_remove(p_list->p_htab, p_iter->key, p_iter->key_len);
        assert(p_remove == p_iter);

        KLB_FREE(p_iter);
        return p_data;
    }

    return NULL;
}

void* klb_hlist_head(klb_hlist_t* p_list)
{
    assert(NULL != p_list);

    return (NULL != p_list->p_head) ? p_list->p_head->p_data : NULL;
}

void* klb_hlist_tail(klb_hlist_t* p_list)
{
    assert(NULL != p_list);

    return (NULL != p_list->p_tail) ? p_list->p_tail->p_data : NULL;
}

uint32_t klb_hlist_size(klb_hlist_t* p_list)
{
    assert(NULL != p_list);

    return p_list->size;
}

void* klb_hlist_remove(klb_hlist_t* p_list, klb_hlist_iter_t* p_iter)
{
    assert(NULL != p_list);

    if (NULL != p_iter)
    {
        klb_hlist_iter_t* p_prev = p_iter->p_prev;
        klb_hlist_iter_t* p_next = p_iter->p_next;

        if (NULL != p_prev)
        {
            p_prev->p_next = p_next;

            if (NULL != p_next)
            {
                p_next->p_prev = p_prev;
            }
            else
            {
                p_list->p_tail = p_prev;
            }
        }
        else
        {
            p_list->p_head = p_next;

            if (NULL != p_next)
            {
                p_next->p_prev = NULL;
            }
            else
            {
                assert(1 == p_list->size);
                p_list->p_tail = NULL;
            }
        }

        p_list->size -= 1;
        assert(0 <= p_list->size);

        void* p_data = p_iter->p_data;
        void* p_remove = klb_htab_remove(p_list->p_htab, p_iter->key, p_iter->key_len);
        assert(p_remove == p_iter);

        KLB_FREE(p_iter);
        return p_data;
    }

    return NULL;
}

void* klb_hlist_data(klb_hlist_iter_t* p_iter)
{
    assert(NULL != p_iter);

    return (NULL != p_iter) ? p_iter->p_data : NULL;
}

klb_hlist_iter_t* klb_hlist_begin(klb_hlist_t* p_list)
{
    assert(NULL != p_list);

    return p_list->p_head;
}

klb_hlist_iter_t* klb_hlist_next(klb_hlist_iter_t* p_iter)
{
    assert(NULL != p_iter);

    return p_iter->p_next;
}

klb_hlist_iter_t* klb_hlist_end(klb_hlist_t* p_list)
{
    assert(NULL != p_list);

    return p_list->p_tail;
}

klb_hlist_iter_t* klb_hlist_prev(klb_hlist_iter_t* p_iter)
{
    assert(NULL != p_iter);

    return p_iter->p_prev;
}

void* klb_hlist_key(klb_hlist_iter_t* p_iter, uint32_t* p_key_len)
{
    assert(NULL != p_iter);
    assert(NULL != p_key_len);

    *p_key_len = p_iter->key_len;

    return (void*)p_iter->key;
}

void* klb_hlist_update(klb_hlist_t* p_list, const void* p_key, uint32_t key_len, void* p_data)
{
    assert(NULL != p_list);
    assert(NULL != p_key);

    klb_hlist_iter_t* p_iter = (klb_hlist_iter_t*)klb_htab_find(p_list->p_htab, p_key, key_len);
    if (NULL != p_iter)
    {
        void* p_old = p_iter->p_data;
        p_iter->p_data = p_data;

        return p_old;
    }
    
    return NULL;
}

klb_hlist_iter_t* klb_hlist_find_iter(klb_hlist_t* p_list, const void* p_key, uint32_t key_len)
{
    assert(NULL != p_list);
    assert(NULL != p_key);

    klb_hlist_iter_t* p_iter = (klb_hlist_iter_t*)klb_htab_find(p_list->p_htab, p_key, key_len);

    return p_iter;
}

void* klb_hlist_find(klb_hlist_t* p_list, const void* p_key, uint32_t key_len)
{
    assert(NULL != p_list);
    assert(NULL != p_key);

    klb_hlist_iter_t* p_iter = (klb_hlist_iter_t*)klb_htab_find(p_list->p_htab, p_key, key_len);

    return (NULL != p_iter) ? p_iter->p_data : NULL;
}

void* klb_hlist_remove_bykey(klb_hlist_t* p_list, const void* p_key, uint32_t key_len)
{
    assert(NULL != p_list);
    assert(NULL != p_key);

    klb_hlist_iter_t* p_iter = klb_hlist_find_iter(p_list, p_key, key_len);

    return klb_hlist_remove(p_list, p_iter);
}

static int cb_hlist_qsort(void* p_arg, void const* p1, void const* p2)
{
    klb_hlist_iter_t** p_iter1 = (klb_hlist_iter_t**)p1;
    klb_hlist_iter_t** p_iter2 = (klb_hlist_iter_t**)p2;

    // 默认按关键字长度小到大, memcmp小到大
    if ((*p_iter1)->key_len < (*p_iter2)->key_len)
    {
        return -1;
    }
    else if ((*p_iter1)->key_len == (*p_iter2)->key_len)
    {
        return memcmp((*p_iter1)->key, (*p_iter2)->key, (*p_iter1)->key_len);
    }

    return 1;
}

void klb_hlist_qsort(klb_hlist_t* p_list)
{
    assert(NULL != p_list);

    uint32_t size = p_list->size;
    if (size <= 1)
    {
        return; // 无需排序
    }

    klb_hlist_iter_t** p_src = KLB_MALLOC(klb_hlist_iter_t*, size, 0);
    
    klb_hlist_iter_t** ptr = p_src;
    klb_hlist_iter_t* p_iter = p_list->p_head;
    while (NULL != p_iter)
    {
        *ptr = p_iter;
        ptr += 1;

        p_iter = p_iter->p_next;
    }

    //qsort(p_src, size, sizeof(klb_hlist_iter_t*), cb_hlist_qsort);
    //qsort_s(p_src, size, sizeof(klb_hlist_iter_t*), cb_hlist_qsort, NULL);

    for (uint32_t i = 1; i < size - 1; i++)
    {
        p_src[i]->p_prev = p_src[i - 1];
        p_src[i]->p_next = p_src[i + 1];
    }

    // 第一个元素
    p_src[0]->p_prev = NULL;
    p_src[0]->p_next = p_src[1];

    // 最后一个元素
    assert(2 <= size);
    p_src[size - 1]->p_prev = p_src[size - 2];
    p_src[size - 1]->p_next = NULL;

    // 头, 尾 指针
    p_list->p_head = p_src[0];
    p_list->p_tail = p_src[size - 1];

    KLB_FREE(p_src);
}
