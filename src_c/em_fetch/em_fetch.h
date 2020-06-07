#ifndef __EM_FETCH_H__
#define __EM_FETCH_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif


typedef struct mnp_fetch_t_ mnp_fetch_t;

mnp_fetch_t* mnp_fetch_create();
void mnp_fetch_destroy(mnp_fetch_t* p_fetch);


int mnp_fetch_request(mnp_fetch_t* p_fetch, const char* p_method, const char* p_url);


#ifdef __cplusplus
}
#endif


#endif // __EM_FETCH_H__
//end
