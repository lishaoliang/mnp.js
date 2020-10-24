// select支持N个socket, 必须在引用select之前定义
#define FD_SETSIZE      1024

#include "em_socket_manage.h"
#include "mem/klb_mem.h"
#include "hash/klb_hlist.h"
#include "em_util/em_log.h"
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <assert.h>


typedef struct em_socket_manage_t_
{
    klb_hlist_t*        p_socket_hlist;     ///< 所有BSD连接列表
    klb_hlist_t*        p_ws_socket_hlist;  ///< 所有websocket连接列表 
    int64_t             tickcount;
}em_socket_manage_t;


em_socket_manage_t* em_socket_manage_create()
{
    em_socket_manage_t* p_manage = KLB_MALLOC(em_socket_manage_t, 1, 0);
    KLB_MEMSET(p_manage, 0, sizeof(em_socket_manage_t));

    p_manage->p_socket_hlist = klb_hlist_create(EM_SOCKET_HASH_TABLE_MAX);
    p_manage->p_ws_socket_hlist = klb_hlist_create(EM_SOCKET_HASH_TABLE_MAX);

    return p_manage;
}

void em_socket_manage_destroy(em_socket_manage_t* p_manage)
{
    assert(NULL != p_manage);

    while (0 < klb_hlist_size(p_manage->p_ws_socket_hlist))
    {
        emws_socket_t* p_emws = (emws_socket_t*)klb_hlist_pop_head(p_manage->p_ws_socket_hlist);
        KLB_FREE_BY(p_emws, emws_socket_destroy);
    }

    while (0 < klb_hlist_size(p_manage->p_socket_hlist))
    {
        em_socket_t* p_socket = (em_socket_t*)klb_hlist_pop_head(p_manage->p_socket_hlist);
        KLB_FREE_BY(p_socket, em_socket_destroy);
    }

    KLB_FREE_BY(p_manage->p_ws_socket_hlist, klb_hlist_destroy);
    KLB_FREE_BY(p_manage->p_socket_hlist, klb_hlist_destroy);
    KLB_FREE(p_manage);
}

int em_socket_manage_push(em_socket_manage_t* p_manage, const void* p_key, uint32_t key_len, em_socket_t* p_socket)
{
    assert(NULL != p_manage);

    int ret = klb_hlist_push_tail(p_manage->p_socket_hlist, p_key, key_len, p_socket);

    if (0 == ret)
    {
        p_socket->tickcount = p_manage->tickcount;
    }

    return ret;
}

int em_socket_manage_push_ws(em_socket_manage_t* p_manage, const void* p_key, uint32_t key_len, emws_socket_t* p_emws)
{
    assert(NULL != p_manage);

    int ret = klb_hlist_push_tail(p_manage->p_ws_socket_hlist, p_key, key_len, p_emws);

    if (0 == ret)
    {
        p_emws->tickcount = p_manage->tickcount;
    }

    return ret;
}

int em_socket_manage_remove(em_socket_manage_t* p_manage, const void* p_key, uint32_t key_len)
{
    assert(NULL != p_manage);

    // socket
    klb_hlist_iter_t* p_iter = klb_hlist_find_iter(p_manage->p_ws_socket_hlist, p_key, key_len);

    if (NULL != p_iter)
    {
        emws_socket_t* p_emws = (emws_socket_t*)klb_hlist_remove(p_manage->p_ws_socket_hlist, p_iter);
        KLB_FREE_BY(p_emws, emws_socket_destroy);
    }

    // socket
    p_iter = klb_hlist_find_iter(p_manage->p_socket_hlist, p_key, key_len);

    if (NULL != p_iter)
    {
        em_socket_t* p_socket = (em_socket_t*)klb_hlist_remove(p_manage->p_socket_hlist, p_iter);
        KLB_FREE_BY(p_socket, em_socket_destroy);
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////

int em_socket_manage_run(em_socket_manage_t* p_manage, int64_t now_ticks)
{
    assert(NULL != p_manage);

    p_manage->tickcount = now_ticks;

    int socket_num = 0, fd_max = 0;
    fd_set r_fds, w_fds;
    FD_ZERO(&r_fds);
    FD_ZERO(&w_fds);

    // BSD socket
    klb_hlist_iter_t* p_iter = klb_hlist_begin(p_manage->p_socket_hlist);
    while (NULL != p_iter)
    {
        em_socket_t* p_socket = (em_socket_t*)klb_hlist_data(p_iter);

        if (fd_max < p_socket->fd)
        {
            fd_max = p_socket->fd;
        }

        FD_SET(p_socket->fd, &r_fds); // 添加读取所有

        if (EM_SOCKET_WRITE & p_socket->flag)
        {
            FD_SET(p_socket->fd, &w_fds); // 只有写标记的才写入
        }

        socket_num += 1;

        p_iter = klb_hlist_next(p_iter);
    }

    // emscripten websocket
    p_iter = klb_hlist_begin(p_manage->p_ws_socket_hlist);
    while (NULL != p_iter)
    {
        emws_socket_t* p_emws = (emws_socket_t*)klb_hlist_data(p_iter);

        if (fd_max < p_emws->fd)
        {
            fd_max = p_emws->fd;
        }

        // emscripten 的读取, 有浏览器提供的回调函数驱动
        // 这里只检测是否可写
        if (EMWS_SOCKET_OPEN == p_emws->status &&
            (EM_SOCKET_WRITE & p_emws->flag))
        {
            FD_SET(p_emws->fd, &w_fds); // 只有写标记的才写入
        }

        socket_num += 1;

        p_iter = klb_hlist_next(p_iter);
    }

    if (socket_num <= 0)
    {
        return 0;
    }

    struct timeval time_out;
    time_out.tv_sec = 0;
    time_out.tv_usec = 1000; // 1ms

    //LOG("fd_max:%d", fd_max);
    int ret = select(fd_max + 1, &r_fds, &w_fds, NULL, &time_out);

    if (0 < ret)
    {
        // BSD socket 遍历列表, 判定读写
        p_iter = klb_hlist_begin(p_manage->p_socket_hlist);
        while (NULL != p_iter)
        {
            em_socket_t* p_socket = (em_socket_t*)klb_hlist_data(p_iter);

            if (FD_ISSET(p_socket->fd, &r_fds) && NULL != p_socket->cb_recv)
            {
                int recv = p_socket->cb_recv(p_socket, now_ticks);

                if (0 < recv)
                {
                    p_socket->tickcount = now_ticks; // 有数据读, 更新时间
                }
            }

            if (FD_ISSET(p_socket->fd, &w_fds) && NULL != p_socket->cb_send)
            {
                int send = p_socket->cb_send(p_socket, now_ticks);

                if (0 < send)
                {
                    p_socket->tickcount = now_ticks; // 有数据写, 更新时间
                }
            }

            p_iter = klb_hlist_next(p_iter);
        }

        // emscripten websocket 遍历列表, 判定写
        p_iter = klb_hlist_begin(p_manage->p_ws_socket_hlist);
        while (NULL != p_iter)
        {
            emws_socket_t* p_emws = (emws_socket_t*)klb_hlist_data(p_iter);

            if (FD_ISSET(p_emws->fd, &w_fds) && NULL != p_emws->cb_send)
            {
                int send = p_emws->cb_send(p_emws, now_ticks);

                if (0 < send)
                {
                    p_emws->tickcount = now_ticks; // 有数据写, 更新时间
                }
            }

            p_iter = klb_hlist_next(p_iter);
        }
    }
    else if (0 == ret)
    {
        // 没有可读写的socket, 还需要等待
    }
    else
    {
        // select失败, 没有数据可读
        assert(false);
    }

    return 0;
}
