#include "em_util/em_buf_mnp.h"
#include "mem/klb_mem.h"
#include "em_util/em_log.h"
#include <assert.h>


#define EM_BUF_MNP_PACKET_SIZE  (30 * 1024)     ///< 30K


int em_buf_mnp_check(const em_buf_t* p_buf)
{
    bool first = true;
    em_buf_t* p_cur = (em_buf_t*)p_buf;
    while (NULL != p_cur)
    {
        klb_mnp_t* p_tmp_mnp = (klb_mnp_t*)p_cur->p_buf;

        assert(KLB_MNP_MAGIC == p_tmp_mnp->magic);

        assert(KLB_MNP_HEART == p_tmp_mnp->packtype ||
            KLB_MNP_TXT == p_tmp_mnp->packtype ||
            KLB_MNP_BIN == p_tmp_mnp->packtype ||
            KLB_MNP_MEDIA == p_tmp_mnp->packtype);

        if (first)
        {
            if (NULL == p_cur->p_next)
            {
                assert(KLB_MNP_FULL == p_tmp_mnp->opt);
            }
            else
            {
                assert(KLB_MNP_BEGIN == p_tmp_mnp->opt);
            }

            first = false;
        }
        else
        {
            if (NULL == p_cur->p_next)
            {
                assert(KLB_MNP_END == p_tmp_mnp->opt);
            }
            else
            {
                assert(KLB_MNP_CONTINUE == p_tmp_mnp->opt);
            }
        }

        assert(p_tmp_mnp->size == p_cur->end - p_cur->start);

        p_cur = p_cur->p_next;
    }

    return 0;
}

int em_buf_mnp_join_md(const em_buf_t* p_buf, klb_mnp_media_t* p_out_md, char** p_data, int* p_data_len)
{
    assert(NULL != p_buf);
    assert(NULL != p_data);

    assert(0 == em_buf_mnp_check(p_buf));

    int buf_len = 0;
    em_buf_t* p_cur = (em_buf_t*)p_buf;
    while (NULL != p_cur)
    {
        buf_len += (p_cur->end - p_cur->start);
        p_cur = p_cur->p_next;
    }

    if (buf_len <= 0)
    {
        return 1;
    }

    char* ptr = KLB_MALLOC(char, buf_len, 0);
    char* p = ptr;
    int data_len = 0;

    bool first = true;
    int size = 0;

    p_cur = (em_buf_t*)p_buf;
    while (NULL != p_cur)
    {
        char* p_src = NULL;
        int src_len = 0;

        klb_mnp_t* p_mnp = (klb_mnp_t*)p_cur->p_buf;

        if (first)
        {
            first = false;

            p_src = (char*)p_cur->p_buf + p_cur->start + sizeof(klb_mnp_t) + sizeof(klb_mnp_media_t);
            src_len = p_cur->end - (p_cur->start + sizeof(klb_mnp_t) + sizeof(klb_mnp_media_t));

            klb_mnp_media_t* p_mnp_md = (klb_mnp_media_t*)(p_cur->p_buf + p_cur->start + sizeof(klb_mnp_t));
            size = p_mnp_md->size;

            if (NULL != p_out_md)
            {
                memcpy(p_out_md, p_mnp_md, sizeof(klb_mnp_media_t));
            }

            assert(src_len == p_mnp->size - sizeof(klb_mnp_t) - sizeof(klb_mnp_media_t));
        }
        else
        {
            p_src = (char*)p_cur->p_buf + p_cur->start + sizeof(klb_mnp_t);
            src_len = p_cur->end - (p_cur->start + sizeof(klb_mnp_t));

            assert(src_len == p_mnp->size - sizeof(klb_mnp_t));
        }

        if (0 < src_len)
        {
            memcpy(p, p_src, src_len);
            p += src_len;
            data_len += src_len;
        } 
        else
        {
            assert(false);
        }

        p_cur = p_cur->p_next;
    }

    assert(data_len + sizeof(klb_mnp_media_t) == size);

    *p_data = ptr;
    if (NULL != p_data_len)
    {
        *p_data_len = data_len;
    }

    return 0;
}

em_buf_t* em_buf_mnp_join_md2(const em_buf_t* p_buf)
{
    assert(NULL != p_buf);

    assert(0 == em_buf_mnp_check(p_buf));

    int buf_len = 0;
    em_buf_t* p_cur = (em_buf_t*)p_buf;
    while (NULL != p_cur)
    {
        buf_len += (p_cur->end - p_cur->start);
        p_cur = p_cur->p_next;
    }

    if (buf_len <= 0)
    {
        return NULL;
    }

    em_buf_t* p_out = em_buf_malloc_ref(buf_len);
    char* ptr = (char*)p_out->p_buf;
    char* p = ptr + sizeof(klb_mnp_media_t);
    int data_len = sizeof(klb_mnp_media_t);

    bool first = true;
    int size = 0;

    p_cur = (em_buf_t*)p_buf;
    while (NULL != p_cur)
    {
        char* p_src = NULL;
        int src_len = 0;

        klb_mnp_t* p_mnp = (klb_mnp_t*)p_cur->p_buf;

        if (first)
        {
            first = false;

            p_src = (char*)p_cur->p_buf + p_cur->start + sizeof(klb_mnp_t) + sizeof(klb_mnp_media_t);
            src_len = p_cur->end - (p_cur->start + sizeof(klb_mnp_t) + sizeof(klb_mnp_media_t));

            klb_mnp_media_t* p_mnp_md = (klb_mnp_media_t*)(p_cur->p_buf + p_cur->start + sizeof(klb_mnp_t));
            size = p_mnp_md->size;

            memcpy(ptr, p_mnp_md, sizeof(klb_mnp_media_t));

            assert(src_len == p_mnp->size - sizeof(klb_mnp_t) - sizeof(klb_mnp_media_t));
        }
        else
        {
            p_src = (char*)p_cur->p_buf + p_cur->start + sizeof(klb_mnp_t);
            src_len = p_cur->end - (p_cur->start + sizeof(klb_mnp_t));

            assert(src_len == p_mnp->size - sizeof(klb_mnp_t));
        }

        if (0 < src_len)
        {
            memcpy(p, p_src, src_len);
            p += src_len;
            data_len += src_len;
        }
        else
        {
            assert(false);
        }

        p_cur = p_cur->p_next;
    }

    assert(data_len == size);
    p_out->end = data_len;

    return p_out;
}

int em_buf_mnp_join_txt(const em_buf_t* p_buf, klb_mnp_common_t* p_out_mnp_txt, char** p_data, int* p_data_len)
{
    assert(NULL != p_buf);
    assert(NULL != p_data);

    assert(0 == em_buf_mnp_check(p_buf));

    int buf_len = 0;
    em_buf_t* p_cur = (em_buf_t*)p_buf;
    while (NULL != p_cur)
    {
        buf_len += (p_cur->end - p_cur->start);
        p_cur = p_cur->p_next;
    }

    if (buf_len <= 0)
    {
        return 1;
    }

    char* ptr = KLB_MALLOC(char, buf_len, 0);
    char* p = ptr;
    int data_len = 0;

    bool first = true;
    int size = 0;

    p_cur = (em_buf_t*)p_buf;
    while (NULL != p_cur)
    {
        char* p_src = NULL;
        int src_len = 0;

        klb_mnp_t* p_mnp = (klb_mnp_t*)p_cur->p_buf;

        if (first)
        {
            first = false;

            p_src = (char*)p_cur->p_buf + p_cur->start + sizeof(klb_mnp_t) + sizeof(klb_mnp_common_t);
            src_len = p_cur->end - (p_cur->start + sizeof(klb_mnp_t) + sizeof(klb_mnp_common_t));

            klb_mnp_common_t* p_mnp_txt = (klb_mnp_common_t*)(p_cur->p_buf + p_cur->start + sizeof(klb_mnp_t));
            size = p_mnp_txt->size;

            if (NULL != p_out_mnp_txt)
            {
                memcpy(p_out_mnp_txt, p_mnp_txt, sizeof(klb_mnp_common_t));
            }

            assert(src_len == p_mnp->size - sizeof(klb_mnp_t) - sizeof(klb_mnp_common_t));
        }
        else
        {
            p_src = (char*)p_cur->p_buf + p_cur->start + sizeof(klb_mnp_t);
            src_len = p_cur->end - (p_cur->start + sizeof(klb_mnp_t));

            assert(src_len == p_mnp->size - sizeof(klb_mnp_t));
        }

        if (0 < src_len)
        {
            memcpy(p, p_src, src_len);
            p += src_len;
            data_len += src_len;
        }
        else
        {
            assert(false);
        }

        p_cur = p_cur->p_next;
    }

    assert(data_len + sizeof(klb_mnp_common_t) == size);

    // 末尾置0
    ptr[data_len] = 0;

    *p_data = ptr;
    if (NULL != p_data_len)
    {
        *p_data_len = data_len;
    }

    return 0;
}

em_buf_t* em_buf_mnp_pack_txt(const char* p_txt, uint32_t sequence, uint32_t uid)
{
    int playload_max = EM_BUF_MNP_PACKET_SIZE;  // 2 ^ 16 - sizeof(klb_mnp_t) - sizeof(klb_mnp_txt_t);

    int data_len = strlen(p_txt);
    char* ptr = (char*)p_txt;

    em_buf_t* p_head = NULL;
    bool first = true;
    int len = data_len;

    while (0 < len)
    {
        int pack_len = MIN(len, playload_max);
        em_buf_t* p_tmp = em_buf_malloc_ref(pack_len + sizeof(klb_mnp_t) + sizeof(klb_mnp_common_t));

        if (first)
        {
            // 数据
            memcpy(p_tmp->p_buf + sizeof(klb_mnp_t) + sizeof(klb_mnp_common_t), ptr, pack_len);

            // klb_mnp_common_t 头
            klb_mnp_common_t* p_mnp_txt = (klb_mnp_common_t*)(p_tmp->p_buf + sizeof(klb_mnp_t));
            memset(p_mnp_txt, 0, sizeof(klb_mnp_common_t));

            p_mnp_txt->size = data_len + sizeof(klb_mnp_common_t);
            p_mnp_txt->extra = 0;
            p_mnp_txt->sequence = sequence;
            p_mnp_txt->uid = uid;

            // klb_mnp_t 头
            klb_mnp_t* p_mnp = (klb_mnp_t*)p_tmp->p_buf;
            memset(p_mnp, 0, sizeof(klb_mnp_t));

            p_mnp->magic = KLB_MNP_MAGIC;
            p_mnp->size = pack_len + sizeof(klb_mnp_t) + sizeof(klb_mnp_common_t);
            p_mnp->opt = (len <= pack_len) ? KLB_MNP_FULL : KLB_MNP_BEGIN;
            p_mnp->packtype = KLB_MNP_TXT;

            em_buf_set_pos(p_tmp, 0, p_mnp->size);

            first = false;
        }
        else
        {
            // 数据
            memcpy(p_tmp->p_buf + sizeof(klb_mnp_t), ptr, pack_len);

            // klb_mnp_t 头
            klb_mnp_t* p_mnp = (klb_mnp_t*)p_tmp->p_buf;
            p_mnp->magic = KLB_MNP_MAGIC;
            p_mnp->size = pack_len + sizeof(klb_mnp_t);
            p_mnp->opt = (len <= pack_len) ? KLB_MNP_END : KLB_MNP_CONTINUE;
            p_mnp->packtype = KLB_MNP_TXT;

            em_buf_set_pos(p_tmp, 0, p_mnp->size);
        }

        // 追加数据到末尾
        p_head = em_buf_append(p_head, p_tmp);

        len -= pack_len;
        ptr += pack_len;
    }

    return p_head;
}
