///////////////////////////////////////////////////////////////////////////
//  Copyright(c) 2019, GNU LESSER GENERAL PUBLIC LICENSE Version 3, 29 June 2007
//
/// @file    klb_list.c
/// @author  李绍良
///  \n https://github.com/lishaoliang/klb/blob/master/LICENSE
///  \n https://github.com/lishaoliang/klb
/// @brief   精简双向链表: 非侵入式
///////////////////////////////////////////////////////////////////////////
#include "list/klb_list.h"
#include "mem/klb_mem.h"
#include <assert.h>


/// @struct klb_list_iter_t
/// @brief  list节点 OR 迭代子
typedef struct klb_list_iter_t_
{
    void*               p_data;         ///< 节点数据

    klb_list_iter_t*    p_prev;         ///< 前一个节点
    klb_list_iter_t*    p_next;         ///< 后一个节点
}klb_list_iter_t;


/// @struct klb_list_t
/// @brief  list对象
typedef struct klb_list_t_
{
    klb_list_iter_t*    p_head;         ///< 起始节点
    klb_list_iter_t*    p_tail;         ///< 末尾节点

    uint32_t            size;           ///< 节点成员数目
}klb_list_t;



klb_list_t* klb_list_create()
{
    klb_list_t* p_list = KLB_MALLOC(klb_list_t, 1, 0);
    KLB_MEMSET(p_list, 0, sizeof(klb_list_t));

    return p_list;
}

void klb_list_destroy(klb_list_t* p_list)
{
    assert(NULL != p_list);
    assert(0 == p_list->size);
    assert(NULL == p_list->p_head);
    assert(NULL == p_list->p_tail);

    // 销毁前必须释放所有节点

    KLB_FREE(p_list);
}


void klb_list_clean(klb_list_t* p_list, klb_list_clean_cb cb_clean, void* p_obj)
{
    assert(NULL != p_list);

    while (true)
    {
        void* p_data = klb_list_pop_head(p_list);

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


void klb_list_push_head(klb_list_t* p_list, void* p_data)
{
    assert(NULL != p_list);

    if (NULL != p_data)
    {
        klb_list_iter_t* p_iter = KLB_MALLOC(klb_list_iter_t, 1, 0);

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
    }
}

void klb_list_push_tail(klb_list_t* p_list, void* p_data)
{
    assert(NULL != p_list);

    if (NULL != p_data)
    {
        klb_list_iter_t* p_iter = KLB_MALLOC(klb_list_iter_t, 1, 0);

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
    }
}

void* klb_list_pop_head(klb_list_t* p_list)
{
    assert(NULL != p_list);

    klb_list_iter_t* p_iter = p_list->p_head;

    if (NULL != p_iter)
    {
        assert(0 <= p_list->size);
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

        KLB_FREE(p_iter);
        return p_data;
    }

    return NULL;
}

void* klb_list_pop_tail(klb_list_t* p_list)
{
    assert(NULL != p_list);

    klb_list_iter_t* p_iter = p_list->p_tail;

    if (NULL != p_iter)
    {
        assert(0 <= p_list->size);
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

        KLB_FREE(p_iter);
        return p_data;
    }

    return NULL;
}

void* klb_list_head(klb_list_t* p_list)
{
    assert(NULL != p_list);

    return (NULL != p_list->p_head) ? p_list->p_head->p_data : NULL;
}

void* klb_list_tail(klb_list_t* p_list)
{
    assert(NULL != p_list);

    return (NULL != p_list->p_tail) ? p_list->p_tail->p_data : NULL;
}

uint32_t klb_list_size(klb_list_t* p_list)
{
    assert(NULL != p_list);

    return p_list->size;
}

void* klb_list_remove(klb_list_t* p_list, klb_list_iter_t* p_iter)
{
    assert(NULL != p_list);
    
    if (NULL != p_iter)
    {
        klb_list_iter_t* p_prev = p_iter->p_prev;
        klb_list_iter_t* p_next = p_iter->p_next;

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
        KLB_FREE(p_iter);

        return p_data;
    }

    return NULL;
}

void* klb_list_data(klb_list_iter_t* p_iter)
{
    assert(NULL != p_iter);

    return (NULL != p_iter) ? p_iter->p_data : NULL;
}

klb_list_iter_t* klb_list_begin(klb_list_t* p_list)
{
    assert(NULL != p_list);

    return p_list->p_head;
}

klb_list_iter_t* klb_list_next(klb_list_iter_t* p_iter)
{
    assert(NULL != p_iter);

    return p_iter->p_next;
}

klb_list_iter_t* klb_list_end(klb_list_t* p_list)
{
    assert(NULL != p_list);

    return p_list->p_tail;
}

klb_list_iter_t* klb_list_prev(klb_list_iter_t* p_iter)
{
    assert(NULL != p_iter);

    return p_iter->p_prev;
}
