#ifndef _HANDLER_H_
#define _HANDLER_H_

#define GETUTIME(t) ((t.tv_sec) * 1000000 + (t.tv_usec))
#define GETSTIME(t) (t.tv_sec)
#define GETMTIME(t) ((((t.tv_sec) * 1000000 + (t.tv_usec))) / 1000)

typedef enum _cmd_e
{
    CMD_ECHO = 100101,
    CMD_TEST,
} cmd_e;

#endif  /** !_HANDLE_H_*/
