#include "app_cmd_json.h"
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


int app_cmd_parse_open(app_cmd_open_t* p_op, const char* p_param)
{
    assert(NULL != p_op);

    // {"ip"="127.0.0.1","port"=8000,"protocol"="WS-MNP","path"="/wsmnp"}

    cJSON* p_root = cJSON_Parse(p_param);

    if (NULL == p_root)
    {
        return 1;
    }

    cJSON* p_ip_json = my_cJSON_GetObjectItem(p_root, "ip", cJSON_String);
    cJSON* p_port_json = my_cJSON_GetObjectItem(p_root, "port", cJSON_Number);
    cJSON* p_protocol_json = my_cJSON_GetObjectItem(p_root, "protocol", cJSON_String);
    cJSON* p_path_json = my_cJSON_GetObjectItem(p_root, "path", cJSON_String);

    int ret = 1;
    if (NULL != p_ip_json)
    {
        // ip
        strncpy(p_op->ip, p_ip_json->valuestring, APP_CMD_STR_LEN);

        // port
        p_op->port = (NULL != p_port_json) ? p_port_json->valueint : 80;

        // protocol
        if (NULL != p_protocol_json)
        {
            strncpy(p_op->protocol, p_protocol_json->valuestring, APP_CMD_STR_LEN);
        }
        else
        {
            strncpy(p_op->protocol, "WS-MNP", APP_CMD_STR_LEN);
        }

        // path
        if (NULL != p_path_json)
        {
            strncpy(p_op->path, p_path_json->valuestring, APP_CMD_PATH_LEN);
        }

        ret = 0;
    }


    KLB_FREE_BY(p_root, cJSON_Delete);
    return ret;
}


int app_cmd_parse_control(app_cmd_control_t* p_control, const char* p_param)
{
    return 0;
}
