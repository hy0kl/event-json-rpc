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

    return "未知错误";
}

static void
build_code_msg(cJSON *json, error_code_e code)
{
    cJSON_AddItemToObject(json, RES_CODE, cJSON_CreateNumber(code));
    cJSON_AddItemToObject(json, RES_MSG,  cJSON_CreateString(get_error_message(code)));

    return;
}

static void
rpc_cmd_echo(struct bufferq *bufferq)
{
    cJSON *json = bufferq->response.json;
    cJSON *data = cJSON_CreateObject();

    build_code_msg(json, E_SUCCESS);

    cJSON_AddItemToObject(json, RES_DATA, data);
    cJSON_AddItemToObject(data, "echo_data", cJSON_CreateString(bufferq->request.buf));


    return;
}

static void
rpc_cmd_undefined(struct bufferq *bufferq)
{
    cJSON *json = bufferq->response.json;
    cJSON *data = cJSON_CreateObject();

    build_code_msg(json, E_CMD_UNDEFINED);
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
            rpc_cmd_echo(bufferq);
            break;

        default:
            rpc_cmd_undefined(bufferq);
    }

    bufferq->response.buf      = cJSON_PrintUnformatted(bufferq->response.json);
    bufferq->response.body_len = strlen(bufferq->response.buf);

    return;
}
