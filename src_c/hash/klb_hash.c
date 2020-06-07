#include "hash/klb_hash.h"

/*-
* SPDX-License-Identifier: (BSD-2-Clause-FreeBSD AND RSA-MD)
*
* Copyright (c) 2010, 2013 Zheng Liu <lz@freebsd.org>
* Copyright (c) 2012, Vyacheslav Matyushin
* All rights reserved.
*
* https://github.com/freebsd/freebsd/blob/master/sys/fs/ext2fs/ext2_hash.c
* ext2_legacy_hash(line.181)
static uint32_t
ext2_legacy_hash(const char *name, int len, int unsigned_char)
{
    uint32_t h0, h1 = 0x12A3FE2D, h2 = 0x37ABE8F9;
    multi = 0x6D22F5;
    const unsigned char *uname = (const unsigned char *)name;
    const signed char *sname = (const signed char *)name;
    int val, i;

    for (i = 0; i < len; i++) {
        if (unsigned_char)
            val = (u_int)*uname++;
        else
            val = (int)*sname++;

        h0 = h2 + (h1 ^ (val * multi));
        if (h0 & 0x80000000)
            h0 -= 0x7FFFFFFF;
        h2 = h1;
        h1 = h0;
    }

    return (h1 << 1);
}
*/
uint32_t klb_hash_legacy(const char* name, int len)
{
    uint32_t h0, h1 = 0x12A3FE2D, h2 = 0x37ABE8F9;
    uint32_t multi = 0x6D22F5;
    const unsigned char *uname = (const unsigned char *)name;
    const signed char *sname = (const signed char *)name;
    int val, i;

    for (i = 0; i < len; i++) {
#if 0
        if (unsigned_char)
            val = (u_int)*uname++;
        else
            val = (int)*sname++;
#else
        val = (int)*sname++;
#endif

        h0 = h2 + (h1 ^ (val * multi));
        if (h0 & 0x80000000)
            h0 -= 0x7FFFFFFF;
        h2 = h1;
        h1 = h0;
    }

    return (h1 << 1);
}
