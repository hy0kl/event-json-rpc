/**
 * @describe:
 * @author: Jerry Yang(hy0kle@gmail.com)
 * */

#include "server.h"
#include "handler.h"

static const char *
get_error_message(error_code_e code)
{
    switch (code)
    {
        case E_SUCCESS:
            return "OK";

        case SERVICE_IS_UNAVAILABLE:
            return "后端服务不可用";

        case E_CMD_UNDEFINED:
            return "操作命令不存在";
    }
}

static void
rpc_cmd_undefined(struct bufferq *bufferq)
{
    cJSON *json = bufferq->response.json;
    cJSON *data = cJSON_CreateObject();

    cJSON_AddItemToObject(json, RES_CODE, cJSON_CreateNumber(E_CMD_UNDEFINED));
    cJSON_AddItemToObject(json, RES_MSG, cJSON_CreateString(get_error_message(E_CMD_UNDEFINED)));
    cJSON_AddItemToObject(json, RES_DATA, data);

    return;
}

void
rpc_handler(struct bufferq *bufferq)
{
    int cmd = cJSON_GetObjectItem(bufferq->request.json, "cmd")->valueint;
    logprintf("[cmd: %d]", cmd);

    switch (cmd)
    {
        case CMD_ECHO:
            break;

        default:
            rpc_cmd_undefined(bufferq);
    }

    bufferq->response.buf      = cJSON_PrintUnformatted(bufferq->response.json);
    bufferq->response.body_len = strlen(bufferq->response.buf);

    return;
}
