#include "em_mnp_avcode.h"


enum AVCodecID em_mnp_to_avcodec(klb_mnp_dtype_e id)
{
    switch (id)
    {
    case KLB_MNP_DTYPE_H264:
        return AV_CODEC_ID_H264;
    case KLB_MNP_DTYPE_H265:
        return AV_CODEC_ID_H265;
    case KLB_MNP_DTYPE_AAC:
        return AV_CODEC_ID_AAC;
    default:
        break;
    }

    return AV_CODEC_ID_NONE;
}
