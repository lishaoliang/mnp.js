#include "em_flv_parser.h"
#include "em_util/em_log.h"
#include <assert.h>


#ifndef MNP_READ_BE16
#define MNP_READ_BE16(x)                            \
        ((((const uint8_t*)(x))[0] << 8) |          \
        ((const uint8_t*)(x))[1])
#endif


#ifndef MNP_READ_BE32
#define MNP_READ_BE32(x)                            \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) |   \
              (((const uint8_t*)(x))[1] << 16) |    \
              (((const uint8_t*)(x))[2] <<  8) |    \
              ((const uint8_t*)(x))[3])
#endif


int em_flv_parser_header(em_flv_header_t* p_header, const char* p_data, int data_len)
{
    assert(NULL != p_header);
    assert(NULL != p_data);

    if (data_len < EM_FLV_HEADER_SIZE0_LEN)
    {
        return 1;
    }

    uint8_t* ptr = (uint8_t*)p_data;

    p_header->signature_f = *ptr;       ptr += 1;
    p_header->signature_l = *ptr;       ptr += 1;
    p_header->signature_v = *ptr;       ptr += 1;
    p_header->version = *ptr;           ptr += 1;

    uint8_t flags = *ptr;               ptr += 1;
    p_header->type_flags_video = flags & 0x1;
    p_header->type_flags_audio = (flags >> 3) & 0x1;

    p_header->data_offset = MNP_READ_BE32(ptr);     ptr += 4;

    uint32_t tag_size0 = MNP_READ_BE32(ptr);        ptr += 4;

    //LOG("em flv parser header:[%d,%d,%d,%d]-[%d,%d]-[%d]", p_header->signature_f, p_header->signature_l, p_header->signature_v, p_header->version,
    //    p_header->type_flags_video, p_header->type_flags_audio, 
    //    p_header->data_offset);

    if (0x46 != p_header->signature_f ||
        0x4C != p_header->signature_l ||
        0x56 != p_header->signature_v ||
        EM_FLV_VERSION_1 != p_header->version)
    {
        return -1; // flv头解析失败
    }

    if (0 != tag_size0)
    {
        LOG("C em flv parser header warning,tag size not zero!");
    }

    return 0;
}

int em_flv_parser_tag(em_flv_tag_t* p_tag, const char* p_data, int data_len)
{
    assert(NULL != p_tag);
    assert(NULL != p_data);

    if (data_len < EM_FLV_TAG_LEN)
    {
        return 1;
    }

    uint8_t* ptr = (uint8_t*)p_data;

    uint32_t b1 = MNP_READ_BE32(ptr);       ptr += 4;
    p_tag->data_size = b1 & 0xFFFFFF;
    p_tag->tag_type = (b1 >> 24) & 0x1F;
    p_tag->filter = (b1 >> 29) & 0x1;

    uint32_t b2 = MNP_READ_BE32(ptr);       ptr += 4;
    p_tag->timestamp_extended = b2 & 0xFF;
    p_tag->timestamp = (b2 >> 8) & 0xFFFFFF;

    p_tag->stream_id[0] = *ptr;             ptr += 1;
    p_tag->stream_id[1] = *ptr;             ptr += 1;
    p_tag->stream_id[2] = *ptr;             ptr += 1;


    //LOG("C em flv parser tag:[%d,%d,%d]-[%d,%d]-[%d,%d,%d]", p_tag->filter, p_tag->tag_type, p_tag->data_size,
    //    p_tag->timestamp, p_tag->timestamp_extended,
    //    p_tag->stream_id[0], p_tag->stream_id[1], p_tag->stream_id[2]);

    return 0;
}

int em_flv_parser_tag_video(em_flv_video_info_t* p_info, const char* p_data, int data_len, int length_size_minus_one)
{
    assert(NULL != p_info);
    assert(NULL != p_data);

    uint8_t* ptr = (uint8_t*)p_data;

    // E.4.3.1  VIDEODATA 
    uint8_t video = *ptr;                   ptr += 1;
    p_info->video.codec_id = video & 0xFF;
    p_info->video.frame_type = (video >> 4) & 0xFF;

    if (EM_FLV_CODEC_AVC == p_info->video.codec_id ||
        EM_FLV_CODEC_AVC_H265 == p_info->video.codec_id)
    {
        uint32_t avc = MNP_READ_BE32(ptr);  ptr += 4;
        p_info->avc.composition_time = avc & 0xFFFFFF;
        p_info->avc.avc_packet_type = (avc >> 24) & 0xFF;

        if (EM_FLV_AVC_SEQUENCE_HEADER == p_info->avc.avc_packet_type)
        {

            p_info->adcr.configuration_version = *ptr;  ptr += 1;   // configurationVersion
            p_info->adcr.avc_profile_indication = *ptr; ptr += 1;   // avcProfileIndication
            p_info->adcr.profile_compatibility = *ptr;  ptr += 1;   // profile_compatibility
            p_info->adcr.avc_level_indication = *ptr;   ptr += 1;   // AVCLevelIndication

            uint8_t smo = *ptr;     ptr += 1;                       // lengthSizeMinusOne
            p_info->adcr.length_size_minus_one = (smo & 0x3) + 1;

            if (3 != p_info->adcr.length_size_minus_one && 4 != p_info->adcr.length_size_minus_one)
            {
                LOG("C em flv parser tag video avc error!lengthSizeMinusOne not 3 or 4");
                return -1;
            }

            // vps/sps/pps
            p_info->p_sps_pps = ptr;
            p_info->sps_pps_len = data_len - (ptr - (uint8_t*)p_data);
#if 1
            if (EM_FLV_CODEC_AVC_H265 == p_info->video.codec_id)
            {
                // VPS
                uint8_t num_of_vps = *ptr;                  ptr += 1;   // numOfSequenceParameterSets
                uint8_t vps_count = num_of_vps & 0x1F;
                for (int i = 0; i < vps_count; i++)
                {
                    uint16_t vps_len = MNP_READ_BE16(ptr);  ptr += 2;   // sequenceParameterSetLength
                    if (0 == vps_len)
                    {
                        continue;
                    }

                    //LOG("C em flv parser vps len:[%d]", vps_len);
                    ptr += vps_len;
                }
            }

            // SPS
            uint8_t num_of_sps = *ptr;                  ptr += 1;   // numOfSequenceParameterSets
            uint8_t sps_count = num_of_sps & 0x1F;
            for (int i = 0; i < sps_count; i++)
            {
                uint16_t sps_len = MNP_READ_BE16(ptr);  ptr += 2;   // sequenceParameterSetLength
                if (0 == sps_len)
                {
                    continue;
                }

                //LOG("C em flv parser sps len:[%d]", sps_len);
                ptr += sps_len;
            }

            // PPS
            uint8_t pps_count = *ptr;                   ptr += 1;   // numOfPictureParameterSets
            for (int i = 0; i < pps_count; i++)
            {
                uint16_t pps_len = MNP_READ_BE16(ptr);  ptr += 2;   // pictureParameterSetLength
                if (0 == pps_len)
                {
                    continue;
                }

                //LOG("em flv parser pps len:[%d]", pps_len);
                ptr += pps_len;
            }
#endif
        }
        else if (EM_FLV_AVC_NALU == p_info->avc.avc_packet_type)
        {
            // nalu
            p_info->p_nalu = ptr;
            p_info->nalu_len = data_len - (ptr - (uint8_t*)p_data);
            p_info->nalu_count = 0;

#if 1
            int spare_len = data_len - (ptr - (uint8_t*)p_data);

            if (3 == length_size_minus_one || 4 == length_size_minus_one)
            {
                // NALU
                int offset = 0;
                int size_minus_one = length_size_minus_one; //

                while (offset + 4 < spare_len)
                {
                    uint32_t nalu_size = MNP_READ_BE32(ptr);
                    if (3 == size_minus_one)
                    {
                        nalu_size = nalu_size >> 8;
                        ptr += 3;
                    }
                    else
                    {
                        ptr += 4;
                    }

                    if (spare_len <= nalu_size)
                    {
                        LOG("C em flv parser tag video nalu error!NALU size");
                        return -1;
                    }

                    //LOG("em flv parser nalu size:[%d]", nalu_size);

                    p_info->nalu_count++;
                    ptr += nalu_size;
                    offset += nalu_size + size_minus_one;
                }
            }
            else
            {
                return 1; // NALU数据无法确定
            }
#endif
        }
    }

    return 0;
}


#define ARRAYOF(arr) sizeof(arr)/sizeof(arr[0])

static const int s_frequency[] = { 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350 };

static int mpeg4_aac_audio_frequency_to(uint8_t index)
{
    if (index < 0 || index >= ARRAYOF(s_frequency))
        return 0;
    return s_frequency[index];
}

int em_flv_parser_tag_audio(em_flv_audio_info_t* p_info, const char* p_data, int data_len)
{
    assert(NULL != p_info);
    assert(NULL != p_data);

    uint8_t* ptr = (uint8_t*)p_data;

    // E.4.2.1  AUDIODATA 
    uint8_t audio = *ptr;                   ptr += 1;
    p_info->audio.sound_type = audio & 0x1;
    p_info->audio.sound_size = (audio >> 1) & 0x1;
    p_info->audio.sound_rate = (audio >> 2) & 0x3;
    p_info->audio.sound_format = (audio >> 4) & 0xF;

    if (EM_FLV_AUDIO_AAC == p_info->audio.sound_format)
    {
        p_info->aac_packet_type = *ptr;     ptr += 1;

        if (EM_FLV_AUDIO_AAC_SEQUENCE_HEADER == p_info->aac_packet_type)
        {
            // https://github.com/lucaszanella/orwell/blob/47a6530d30f36d8acf7647cfa3c24285618fd047/deps/ZLMediaKit/3rdpart/media-server/libflv/source/mpeg4-aac.c

            p_info->aac_seq_head.profile = (ptr[0] >> 3) & 0x1F;
            p_info->aac_seq_head.sampling_frequency_index = ((ptr[0] & 0x7) << 1) | ((ptr[1] >> 7) & 0x01);
            p_info->aac_seq_head.channel_configuration = (ptr[1] >> 3) & 0x0F;

            assert(p_info->aac_seq_head.profile > 0 && p_info->aac_seq_head.profile < 31);
            assert(p_info->aac_seq_head.channel_configuration >= 0 && p_info->aac_seq_head.channel_configuration <= 7);
            assert(p_info->aac_seq_head.sampling_frequency_index >= 0 && p_info->aac_seq_head.sampling_frequency_index <= 0xc);

            p_info->aac_seq_head.channels = p_info->aac_seq_head.channel_configuration;
            p_info->aac_seq_head.sampling_frequency = mpeg4_aac_audio_frequency_to(p_info->aac_seq_head.sampling_frequency_index);

            //LOG("C em flv parser tag audio:[%d:%d:%d;%d],[%d],[%d,%d,%d]", p_info->audio.sound_format, p_info->audio.sound_rate, p_info->audio.sound_size, p_info->audio.sound_type,
            //    p_info->aac_packet_type,
            //    p_info->aac_seq_head.profile, p_info->aac_seq_head.channels, p_info->aac_seq_head.sampling_frequency);

        }
        else if (EM_FLV_AUDIO_AAC_RAW == p_info->aac_packet_type)
        {
            p_info->p_aac_raw = ptr;
            p_info->aac_raw_len = data_len - (ptr - (uint8_t*)p_data);

            //LOG("C aac raw : [%d], [%d,%d,%d,%d,%d]", p_info->aac_raw_len,
            //    ptr[0], ptr[1], ptr[2], ptr[3], ptr[4]);
        }
    }

    return 0;
}
