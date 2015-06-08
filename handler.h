#ifndef _HANDLER_H_
#define _HANDLER_H_

typedef enum _cmd_e
{
    CMD_ECHO = 100101,
    CMD_TEST,
} cmd_e;


void rpc_handler(struct bufferq *bufferq);

#endif  /** !_HANDLE_H_*/
