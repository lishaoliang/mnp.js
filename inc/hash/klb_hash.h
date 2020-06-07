#ifndef __KLB_HASH_H__
#define __KLB_HASH_H__

#include "klb_type.h"
#include "hash/klb_hash_hgsmi.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define klb_hash32          klb_hash_legacy


/// @brief legacy hash 算法
/// @param [in] *p_data 待计算hash的指针
/// @param [in] len     长度
/// @return uint32_t hash值
KLB_API uint32_t klb_hash_legacy(const char* name, int len);


#ifdef __cplusplus
}
#endif


#endif // __KLB_HASH_H__
//end
