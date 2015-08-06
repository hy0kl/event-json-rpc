#ifndef _SERVER_H_
#define _SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <err.h>
#include <getopt.h>
#include <netinet/in.h>
#include <sys/types.h>
/* Required by event.h. */
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

/* For inet_ntoa. */
#include <arpa/inet.h>

/* Libevent. */
#include <event.h>

/** c json lib */
#include "cJSON.h"

/** zllog*/
#include <zlog.h>

/* Easy sensible linked lists. */
#include "queue.h"

#define GETUTIME(t) ((t.tv_sec) * 1000000 + (t.tv_usec))
#define GETSTIME(t) (t.tv_sec)
#define GETMTIME(t) ((((t.tv_sec) * 1000000 + (t.tv_usec))) / 1000)

/* Port to listen on. */
#define SERVER_PORT 5555

/** rpc 协议头长度 */
#define PROTOCOL_HEADER_LEN sizeof(int)

#define _DEBUG_ 1

#if (_DEBUG_) /** { */
#define logprintf(format, arg...) fprintf(stderr, "[DEBUG]%s:%d:%s "format"\n", __FILE__, __LINE__, __func__, ##arg)
#else /** } {*/
#define logprintf(format, arg...) {}
#endif /** } */

/**
 _ __ ___   __ _  ___ _ __ ___
| '_ ` _ \ / _` |/ __| '__/ _ \
| | | | | | (_| | (__| | | (_) |
|_| |_| |_|\__,_|\___|_|  \___/
*/
/** 配置项目缓冲区长度 */
#define CONF_BUF_LEN    128
/** 文件路径缓冲区长度 */
#define PATH_BUF_LEN    1024

/* Length of each buffer in the buffer queue.  Also becomes the amount
 * of data we try to read per call to read(2). */
#define BUFLEN 20480

/** 全局状态码/错误码 */
typedef enum _g_error_code_e
{
    CAN_NOT_OPEN_SERVER_JSON    = -11,
    NEED_MORE_MEMORY            = -12,
    JSON_PARSE_FAILURE          = -13,
    CAN_NOT_OPEN_ZLOG_CONF      = -1,
    CAN_NOT_GET_ZLOG_CATEGORY   = -2,
} g_error_code_e;

/** 请求体 */
typedef struct _request_t
{
    int     body_len;   /** 请求体的 body 长度 */

    /* The buffer. */
    char *buf;        /** 请求体的 json 文本 */

    cJSON  *json;       /** 请求体 parse 后的 json 对象 */
} request_t;

/** 响应体 */
typedef struct _response_t
{
    /** 响应体的长度 */
    int     body_len;

    /* The buffer. */
    char *buf;

    /* The offset into buf to start writing from. */
    int     offset;

    cJSON  *json;   /** 响应体组装的 json 对象 */
} response_t;

/**
 * In event based programming we need to queue up data to be written
 * until we are told by libevent that we can write.  This is a simple
 * queue of buffers to be written implemented by a TAILQ from queue.h.
 */
struct bufferq
{
    request_t  request;
    response_t response;

    /* For the linked list structure. */
    TAILQ_ENTRY(bufferq) entries;
};

/**
 * A struct for client specific data, also includes pointer to create
 * a list of clients.
 *
 * In event based programming it is usually necessary to keep some
 * sort of object per client for state information.
 */
struct client {
    /* Events. We need 2 event structures, one for read event
     * notification and the other for writing. */
    struct event ev_read;
    struct event ev_write;

    /* This is the queue of data to be written to this client. As
     * we can't call write(2) until libevent tells us the socket
     * is ready for writing. */
    TAILQ_HEAD(, bufferq) writeq;
};

typedef unsigned int u_int;

typedef struct _mysql_config_t
{
    char   host[CONF_BUF_LEN];
    u_int  port;
    char   dbname[CONF_BUF_LEN];
    char   username[CONF_BUF_LEN];
    char   password[CONF_BUF_LEN];
} mysql_config_t;

/** 全局服务配置结构体 */
typedef struct _server_config_t
{
    u_int daemon;   /** 是否以守护进程方式工作 */
    u_int port;     /** 监听的端口 */

    char zlog_conf[PATH_BUF_LEN];
    char zlog_category[CONF_BUF_LEN];

    mysql_config_t mysql_master;
    mysql_config_t mysql_slaves;
} server_config_t;

/** 函数前置申明 */
void
parse_server_config();

void
init_global_zlog();

int
setnonblock(int fd);

void
on_handler(struct bufferq *bufferq);

/**
 * This function will be called by libevent when the client socket is
 * ready for reading.
 */
void
on_read(int fd, short ev, void *arg);

/**
 * This function will be called by libevent when the client socket is
 * ready for writing.
 */
void
on_write(int fd, short ev, void *arg);

/**
 * This function will be called by libevent when there is a connection
 * ready to be accepted.
 */
void
on_accept(int fd, short ev, void *arg);

/**
 * global variables
 * */
extern server_config_t g_srv_conf;
extern zlog_category_t *g_zc; /** 全局日志句柄 */

#endif
