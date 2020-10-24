#include "app_cmd_json.h"
#include "em_conn_manage.h"
#include "cJSON.h"
#include "mem/klb_mem.h"
#include <stdio.h>
#include <assert.h>

//
static cJSON* my_cJSON_GetObjectItem(const cJSON * const p_object, const char * const p_string, int json_type)
{
    // json_type = cJSON_String, cJSON_Number etc.
    cJSON* p_item = cJSON_GetObjectItem(p_object, p_string);

    if ((NULL != p_item) && ((p_item->type & 0xFF) == json_type))
    {
        return p_item;
    }

    return NULL;
}


int app_cmd_parse_open(app_cmd_open_t* p_open, const char* p_param)
{
    assert(NULL != p_open);

    // {"ip"="127.0.0.1","port"=8000,"protocol"="WS-MNP","path"="/wsmnp"}

    cJSON* p_root = cJSON_Parse(p_param);

    if (NULL == p_root)
    {
        return 1;
    }

    cJSON* p_protocol_json = my_cJSON_GetObjectItem(p_root, "protocol", cJSON_String);
    cJSON* p_url_json = my_cJSON_GetObjectItem(p_root, "url", cJSON_String);

    int ret = 1;
    if (NULL != p_url_json)
    {
        // protocol
        if (NULL != p_protocol_json)
        {
            strncpy(p_open->protocol, p_protocol_json->valuestring, APP_CMD_STR_LEN);
        }
        else
        {
            strncpy(p_open->protocol, EM_NET_WS_MNP, APP_CMD_STR_LEN);
        }

        int url_len = strlen(p_url_json->valuestring);
        p_open->p_url = KLB_MALLOC(char, url_len + 4, 0);
        strcpy(p_open->p_url, p_url_json->valuestring);

        ret = 0;
    }


    KLB_FREE_BY(p_root, cJSON_Delete);
    return ret;
}

void app_cmd_parse_free_open(app_cmd_open_t* p_open)
{
    assert(NULL != p_open);
    if (NULL != p_open)
    {
        KLB_FREE(p_open->p_url);
    }
}


int app_cmd_parse_control(app_cmd_control_t* p_control, const char* p_param)
{
    return 0;
}
