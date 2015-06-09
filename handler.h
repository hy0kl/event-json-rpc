#ifndef _HANDLER_H_
#define _HANDLER_H_

#define RES_CODE    "code"
#define RES_MSG     "message"
#define RES_DATA    "data"

typedef enum _cmd_e
{
    CMD_ECHO = 100101,
    CMD_TEST,
} cmd_e;

typedef enum _error_code_e
{
    E_SUCCESS = 0,
    SERVICE_IS_UNAVAILABLE,
    E_CMD_UNDEFINED,
} error_code_e;

void
rpc_handler(struct bufferq *bufferq);

#endif  /** !_HANDLE_H_*/
