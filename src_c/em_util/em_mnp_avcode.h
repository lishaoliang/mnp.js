#ifndef __EM_MNP_AVCODE_H__
#define __EM_MNP_AVCODE_H__

#include "klb_type.h"
#include "mnp/klb_mnp.h"
#include "libavcodec/avcodec.h"

#if defined(__cplusplus)
extern "C" {
#endif


enum AVCodecID em_mnp_to_avcodec(klb_mnp_dtype_e id);




#ifdef __cplusplus
}
#endif

#endif // __EM_MNP_AVCODE_H__
//end
