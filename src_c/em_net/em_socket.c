#include "em_socket.h"
#include "mem/klb_mem.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


static int close_socket(em_socket_fd fd)
{
    int ret = close(fd);
    return ret;
}

static int open_socket(int af, int type, int protocol)
{
    em_socket_fd fd = socket(af, type, protocol);
    if (INVALID_SOCKET == fd)
    {
        return INVALID_SOCKET;
    }

    // 设置非阻塞方式
    int getfl = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, getfl | O_NONBLOCK);

    return fd;
}


em_socket_t* em_socket_create(int af, int type, int protocol)
{
    em_socket_fd fd = open_socket(af, type, protocol);
    if (INVALID_SOCKET == fd)
    {
        return NULL;
    }

    em_socket_t* p_socket = KLB_MALLOC(em_socket_t, 1, 0);
    KLB_MEMSET(p_socket, 0, sizeof(em_socket_t));

    p_socket->fd = fd;

    return p_socket;
}

void em_socket_destroy(em_socket_t* p_socket)
{
    assert(NULL != p_socket);

    if (INVALID_SOCKET != p_socket->fd)
    {
        close_socket(p_socket->fd);
        p_socket->fd = INVALID_SOCKET;
    }

    KLB_FREE(p_socket);
}

// 请求:
//GET / HTTP/1.1
//Host: 127.0.0.1:8000
//User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:75.0) Gecko/20100101 Firefox/75.0
//Accept: */*
//Accept-Language: zh-CN,zh;q=0.8,zh-TW;q=0.7,zh-HK;q=0.5,en-US;q=0.3,en;q=0.2
//Accept-Encoding: gzip, deflate
//Sec-WebSocket-Version: 13
//Origin: http://127.0.0.1:8888
//Sec-WebSocket-Protocol: binary
//Sec-WebSocket-Extensions: permessage-deflate
//Sec-WebSocket-Key: CDnYx0Byn2TFp1yiO8o7jQ==
//Connection: keep-alive, Upgrade
//Cookie: lang=en_us
//Pragma: no-cache
//Cache-Control: no-cache
//Upgrade: websocket
// 
// 回复:
//HTTP/1.1 101 Switching Protocols
//Server: KLB
//Upgrade: websocket
//Connection: Upgrade
//Sec-WebSocket-Version: 13
//Sec-WebSocket-Accept: x+kKuM/woLF6Rsww9tYbwvjnry0=
//Sec-WebSocket-Protocol: binary
//Content-Length: 0
//Access-Control-Allow-Origin: *
int em_socket_connect(em_socket_t* p_socket, const char* p_ip, int port)
{
    assert(NULL != p_socket);

    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(p_ip);
    addr.sin_port = htons(port);

    if (0 != connect(p_socket->fd, (struct sockaddr*)&addr, sizeof(addr)))
    {
        if (EISCONN == errno)
        {
            return 0;
        }
        else if ((EINPROGRESS != errno) && (EWOULDBLOCK != errno) && (EAGAIN != errno) && (EALREADY != errno))
        {
            return 1;
        }
    }

    return 0;
}

int em_socket_send(em_socket_t* p_socket, const uint8_t* p_buf, int len)
{
    assert(NULL != p_socket);

    int ret = send(p_socket->fd, p_buf, len, MSG_DONTWAIT | MSG_NOSIGNAL);

    return ret;
}

int em_socket_recv(em_socket_t* p_socket, uint8_t* p_buf, int len)
{
    assert(NULL != p_socket);

    int ret = recv(p_socket->fd, p_buf, len, MSG_DONTWAIT);

    return ret;
}

int em_socket_sendto(em_socket_t* p_socket, const uint8_t* p_buf, int len, const struct sockaddr* p_addr, int addr_len)
{
    assert(NULL != p_socket);

    int ret = sendto(p_socket->fd, p_buf, len, MSG_DONTWAIT | MSG_NOSIGNAL, p_addr, addr_len);

    return ret;
}

int em_socket_recvfrom(em_socket_t* p_socket, uint8_t* p_buf, int len, struct sockaddr* p_addr, int* p_addr_len)
{
    assert(NULL != p_socket);

    socklen_t addr_len = *p_addr_len;
    int ret = recvfrom(p_socket->fd, p_buf, len, MSG_DONTWAIT, p_addr, &addr_len);

    *p_addr_len = addr_len;

    return ret;
}

void em_socket_set_flag_write(em_socket_t* p_socket)
{
    p_socket->flag |= EM_SOCKET_WRITE;
}

void em_socket_clear_flag_write(em_socket_t* p_socket)
{
    p_socket->flag &= ~(uint32_t)(EM_SOCKET_WRITE);
}
