/**
 * @describe:
 * @author: Jerry Yang(hy0kle@gmail.com)
 * */

#include "server.h"
#include "handler.h"

void rpc_handler(struct bufferq *bufferq)
{
    const char *strings[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    bufferq->response.json = cJSON_CreateStringArray(strings, 7);
    bufferq->response.buf  = cJSON_PrintUnformatted(bufferq->response.json);
    bufferq->response.body_len = strlen(bufferq->response.buf);

    return;
}
