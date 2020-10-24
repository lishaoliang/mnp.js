#ifndef __EM_FLV_PARSER_H__
#define __EM_FLV_PARSER_H__

#include "klb_type.h"

#if defined(__cplusplus)
extern "C" {
#endif

#pragma pack(1) /// 1 字节对齐

/// video_file_format_spec_v10_1.pdf

/// E.2  The FLV header 
typedef struct em_flv_header_t_
{
#define EM_FLV_HEADER_LEN        9      ///< FLV头部长度
#define EM_FLV_HEADER_SIZE0_LEN  13     ///< FLV头 + 第一个PreviousTagSize0

    uint8_t signature_f;                ///< Signature byte always 'F' (0x46) 
    uint8_t signature_l;                ///< Signature byte always 'L' (0x4C) 
    uint8_t signature_v;                ///< Signature byte always 'V' (0x56) 
    uint8_t version;                    ///< File version (for example, 0x01 for FLV version 1) 
#define EM_FLV_VERSION_1    0x1         ///< 版本1

    uint8_t type_flags_reserved1 : 5;   ///< Shall be 0 
    uint8_t type_flags_audio : 1;       ///< 1 = Audio tags are present 
    uint8_t type_flags_reserved2 : 1;   ///< Shall be 0 
    uint8_t type_flags_video : 1;       ///< 1 = Video tags are present 

    uint32_t data_offset;               ///< The length of this header in bytes = 9 
}em_flv_header_t;


/// E.4.1  FLV Tag  
typedef struct em_flv_tag_t_
{
#define EM_FLV_TAG_LEN     11           ///< tag头长度 

    uint32_t reserved1 : 2;             ///< Reserved for FMS, should be 0 
    uint32_t filter : 1;                ///< Indicates if packets are filtered. 0 = No pre - processing required.1 = Pre - processing(such as decryption) of the packet
    uint32_t tag_type : 5;              ///< tag类型
#define EM_FLV_TAG_AUDIO    8           ///< 音频
#define EM_FLV_TAG_VIDEO    9           ///< 视频
#define EM_FLV_TAG_SCRIPT   18          ///< 脚本

    uint32_t data_size : 24;


    uint32_t timestamp : 24;            ///< 时间戳
    uint32_t timestamp_extended : 8;    ///< 扩展时间戳

    uint8_t  stream_id[3];              ///< Always 0
}em_flv_tag_t;


/// E.4.3.1  VIDEODATA
typedef struct em_flv_video_tag_t_
{
    uint8_t frame_type : 4;
    uint8_t codec_id : 4;

#define EM_FLV_AVC_KEY_FRAME    1       ///< AVC关键帧
#define EM_FLV_AVC_INTER_FRAME  2       ///< AVC非关键帧

#define EM_FLV_CODEC_AVC        7       ///< AVC
#define EM_FLV_CODEC_AVC_H265   12      ///< AVC(h265); 非标准
}em_flv_video_tag_t;


/// E.4.3.1  VIDEODATA
typedef struct em_flv_avc_t_
{
    uint32_t avc_packet_type : 8;
    int32_t  composition_time : 24;     ///< avc_packet_type = EM_FVL_AVC_NALU有效

#define EM_FLV_AVC_SEQUENCE_HEADER  0
#define EM_FLV_AVC_NALU             1
#define EM_FLV_AVC_SEQUENCE_END     2
}em_flv_avc_t;


/// AVCDecoderConfigurationRecord
typedef struct em_flv_avc_decoder_configuration_record_t_
{
    uint8_t   configuration_version;    ///< configurationVersion
    uint8_t   avc_profile_indication;   ///< avcProfileIndication
    uint8_t   profile_compatibility;    ///< profile_compatibility
    uint8_t   avc_level_indication;     ///< AVCLevelIndication
    uint8_t   length_size_minus_one;    ///< lengthSizeMinusOne
}em_flv_avc_decoder_configuration_record_t;


/// E.4.2.1  AUDIODATA 
typedef struct em_flv_audio_tag_t_
{
    uint8_t sound_format : 4;
#define EM_FLV_AUDIO_LPCM_PE        0   ///< Linear PCM, platform endian
#define EM_FLV_AUDIO_ADPCM          1   ///< ADPCM
#define EM_FLV_AUDIO_MP3            2   ///< MP3
#define EM_FLV_AUDIO_AAC            10  ///< AAC

    uint8_t sound_rate : 2;
#define EM_FLV_AUDIO_5DOT5_KHZ      0   ///< 5.5kHz
#define EM_FLV_AUDIO_11_KHZ         1   ///< 11kHz
#define EM_FLV_AUDIO_22_KHZ         2   ///< 22kHz
#define EM_FLV_AUDIO_44_KHZ         3   ///< 44kHz

    uint8_t sound_size : 1;
#define EM_FLV_AUDIO_SAMPLES_8BIT   0   ///<  8-bit samples
#define EM_FLV_AUDIO_SAMPLES_16BIT  1   ///<  16-bit samples

    uint8_t sound_type : 1;
#define EM_FLV_AUDIO_MONO           0   ///< Mono sound
#define EM_FLV_AUDIO_STEREO         1   ///< Stereo sound
}em_flv_audio_tag_t;


#define EM_FLV_AUDIO_AAC_SEQUENCE_HEADER    0   ///< AAC sequence header
#define EM_FLV_AUDIO_AAC_RAW                1   ///< AAC sequence header

/// AudioSpecificConfiguration
typedef struct em_flv_aac_sequence_header_t_
{
    uint8_t profile; // 0-NULL, 1-AAC Main, 2-AAC LC, 2-AAC SSR, 3-AAC LTP
    uint8_t sampling_frequency_index; // 0-96000, 1-88200, 2-64000, 3-48000, 4-44100, 5-32000, 6-24000, 7-22050, 8-16000, 9-12000, 10-11025, 11-8000, 12-7350, 13/14-reserved, 15-frequency is written explictly
    uint8_t channel_configuration; // 0-AOT, 1-1channel,front-center, 2-2channels, front-left/right, 3-3channels: front center/left/right, 4-4channels: front-center/left/right, back-center, 5-5channels: front center/left/right, back-left/right, 6-6channels: front center/left/right, back left/right LFE-channel, 7-8channels

    uint8_t channels; // valid only in decode; 2
    uint32_t sampling_frequency;  // valid only in decode; 44100
}em_flv_aac_sequence_header_t;

#pragma pack()


/// @brief 解析FLV头
/// @param [out] *p_header  FLV头
/// @param [in]  *p_data    数据
/// @param [in]  data_len   数据长度
/// @return int 0.解析成功, 头部长度+第一个PreviousTagSize0 : EM_FVL_HEADER_SIZE0_LEN; 
///  \n         -1.解析失败, 数据错误
///  \n         1. 解析失败, 数长度不足
int em_flv_parser_header(em_flv_header_t* p_header, const char* p_data, int data_len);


/// @brief 解析tag头
/// @param [out] *p_tag     tag头
/// @param [in]  *p_data    数据
/// @param [in]  data_len   数据长度
/// @return int 0.解析成功, 头部长度为: EM_FVL_TAG_LEN; 
///  \n         -1.解析失败, 数据错误
///  \n         1. 解析失败, 数长度不足
int em_flv_parser_tag(em_flv_tag_t* p_tag, const char* p_data, int data_len);



typedef struct em_flv_video_info_t_
{
    em_flv_video_tag_t  video;
    em_flv_avc_t        avc;
    em_flv_avc_decoder_configuration_record_t   adcr;

    uint8_t*    p_sps_pps;          ///< VPS/SPS/PPS
    int         sps_pps_len;        ///< VPS/SPS/PPS数据长度: 含flv长度标记

    uint8_t*    p_nalu;             ///< NALU
    int         nalu_len;           ///< NALU整块数据长度: 含flv长度标记
    int         nalu_count;         ///< NALU块数目
}em_flv_video_info_t;


/// @brief 解析tag video
/// @return int 0.解析成功
///  \n         -1.解析失败, 数据错误
///  \n         1. 解析失败, 数长度不足
int em_flv_parser_tag_video(em_flv_video_info_t* p_info, const char* p_data, int data_len, int length_size_minus_one);


typedef struct em_flv_audio_info_t_
{
    em_flv_audio_tag_t  audio;
    uint8_t             aac_packet_type;            ///< EM_FLV_AUDIO_AAC_SEQUENCE_HEADER, EM_FLV_AUDIO_AAC_RAW

    em_flv_aac_sequence_header_t    aac_seq_head;
    uint8_t*                        p_aac_raw;
    int                             aac_raw_len;
}em_flv_audio_info_t;


int em_flv_parser_tag_audio(em_flv_audio_info_t* p_info, const char* p_data, int data_len);


#ifdef __cplusplus
}
#endif

#endif // __EM_FLV_PARSER_H__
//end
