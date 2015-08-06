/**
 * @describe:
 * @author: Jerry Yang(hy0kle@gmail.com)
 * */

#include "server.h"

/** handler */
#include "handler.h"

void
parse_server_config()
{
    FILE *fp = fopen("conf/server.json", "r");
    if (NULL == fp)
    {
        fprintf(stderr, "无法打开主配置文件: conf/server.json\n");
        exit(CAN_NOT_OPEN_SERVER_JSON);
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *data = (char*)malloc(len + 1);

    if (NULL == data)
    {
        fprintf(stderr, "无法为解析配置文件申请足够的内存. need: %ld\n", len);
        exit(NEED_MORE_MEMORY);
    }

    fread(data, 1, len, fp);
    fclose(fp);

    /** 解析 json 的配置文件 */
    cJSON *root_json = cJSON_Parse(data);
    if (NULL == root_json)
    {
        fprintf(stderr, "解析JSON失败, error:%s\n", cJSON_GetErrorPtr());
        cJSON_Delete(root_json);
        exit(JSON_PARSE_FAILURE);
    }

    /** 是否以守护进程工作 */
    g_srv_conf.daemon = 0;
    int daemon = cJSON_GetObjectItem(root_json, "daemon")->valueint;
    if (daemon)
    {
        g_srv_conf.daemon = daemon;
    }
    //logprintf("g_srv_conf.daemon = %d", g_srv_conf.daemon);

    g_srv_conf.port = SERVER_PORT;  /** 默认监听端口 */
    int port = cJSON_GetObjectItem(root_json, "port")->valueint;
    if (port > 0)
    {
        g_srv_conf.port = (u_int)port;
    }
    //logprintf("g_srv_conf.port = %d\n", g_srv_conf.port);

    /** hard-code 去掉 json 字符串被解析后两边的 " */
    cJSON *zlog_conf = cJSON_GetObjectItem(root_json, "zlog_conf");
    snprintf(g_srv_conf.zlog_conf, CONF_BUF_LEN, "%s", zlog_conf->valuestring);
    logprintf("g_srv_conf.zlog_conf = %s", g_srv_conf.zlog_conf);

    cJSON *zlog_category = cJSON_GetObjectItem(root_json, "zlog_category");
    snprintf(g_srv_conf.zlog_category, CONF_BUF_LEN, "%s", zlog_category->valuestring);
    logprintf("g_srv_conf.zlog_category = %s", g_srv_conf.zlog_category);

    cJSON_Delete(root_json);

    return;
}

void
init_global_zlog()
{
    /** 初始化日志 */
    int rc;
    //logprintf("g_srv_conf.zlog_conf = %s", g_srv_conf.zlog_conf);
    rc = zlog_init(g_srv_conf.zlog_conf);
    if (rc) {
        fprintf(stderr, "zlog init failed\n");
        exit(CAN_NOT_OPEN_ZLOG_CONF);
    }

    g_zc = zlog_get_category(g_srv_conf.zlog_category);
    if (!g_zc) {
        fprintf(stderr, "zlog get cat fail\n");
        zlog_fini();
        exit(CAN_NOT_GET_ZLOG_CATEGORY);
    }
    zlog_info(g_zc, "程序初始化");

    return;
}

/**
 * Set a socket to non-blocking mode.
 */
int
setnonblock(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL);
    if (flags < 0)
        return flags;

    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0)
        return -1;

        return 0;
}

void
on_handler(struct bufferq *bufferq)
{
    rpc_handler(bufferq);
    return;
}

void
on_read(int fd, short ev, void *arg)
{
    struct client *client = (struct client *)arg;
    struct bufferq *bufferq;
    char *req_buf;
    int body_len = 0;
    int len = 0;

    zlog_debug(g_zc, "fd: %d", fd);

    /* Because we are event based and need to be told when we can
     * write, we have to malloc the read buffer and put it on the
     * clients write queue. */
    req_buf = malloc(BUFLEN);
    if (req_buf == NULL) {
        err(1, "malloc failed for request buffer.");
    }

    /** 读协议头 */
    len = read(fd, &body_len, PROTOCOL_HEADER_LEN);
    if (PROTOCOL_HEADER_LEN != len || body_len > BUFLEN) {
        zlog_warn(g_zc, "Protocol header has something wrong. read len: %d, body_len: %d\n", len, body_len);

        goto READ_EXCEPTION;
    }

    zlog_debug(g_zc, "request.body_len: %d", body_len);

    len = read(fd, req_buf, body_len);
    if (len == 0) {
        /* Client disconnected, remove the read event and the
         * free the client structure. */
        zlog_info(g_zc, "Client disconnected.\n");

        goto READ_EXCEPTION;
    }
    else if (len < 0) {
        /* Some other error occurred, close the socket, remove
         * the event and free the client structure. */
        zlog_error(g_zc, "Socket failure, disconnecting client: %s",
            strerror(errno));

        goto READ_EXCEPTION;
    }

    req_buf[body_len] = '\0';   /** 手工将请求的字符串结束 */
    zlog_debug(g_zc, "request-json: %s", req_buf);

    /* We can't just write the buffer back as we need to be told
     * when we can write by libevent.  Put the buffer on the
     * client's write queue and schedule a write event. */
    bufferq = calloc(1, sizeof(*bufferq));
    if (bufferq == NULL) {
        err(1, "malloc faild for bufferq.");
    }

    /** TODO 解析和创建 json 数据时需要容错 */
    bufferq->request.buf  = req_buf;
    bufferq->request.json = cJSON_Parse(req_buf);

    bufferq->response.buf       = NULL;
    bufferq->response.body_len  = 0;
    bufferq->response.offset    = 0;
    bufferq->response.json      = cJSON_CreateObject();

    on_handler(bufferq);

    TAILQ_INSERT_TAIL(&client->writeq, bufferq, entries);

    /* Since we now have data that needs to be written back to the
     * client, add a write event. */
    event_add(&client->ev_write, NULL);
    return;

READ_EXCEPTION:
    close(fd);

    event_del(&client->ev_read);
    free(client);

    free(req_buf);

    return;
}

void
on_write(int fd, short ev, void *arg)
{
    struct client *client = (struct client *)arg;
    struct bufferq *bufferq;
    int len;

    /* Pull the first item off of the write queue. We probably
     * should never see an empty write queue, but make sure the
     * item returned is not NULL. */
    bufferq = TAILQ_FIRST(&client->writeq);
    if (bufferq == NULL)
        return;

    /** 写头协议 */
    if (0 == bufferq->response.offset) {
        write(fd, &(bufferq->response.body_len), PROTOCOL_HEADER_LEN);
    }

    /* Write the buffer.  A portion of the buffer may have been
     * written in a previous write, so only write the remaining
     * bytes. */
    len = bufferq->response.body_len - bufferq->response.offset;
    len = write(fd, bufferq->response.buf + bufferq->response.offset,
                    bufferq->response.body_len - bufferq->response.offset);
    if (len == -1) {
        if (errno == EINTR || errno == EAGAIN) {
            /* The write was interrupted by a signal or we
             * were not able to write any data to it,
             * reschedule and return. */
            event_add(&client->ev_write, NULL);
            return;
        }
        else {
            /* Some other socket error occurred, exit. */
            err(1, "write");
        }
    }
    else if ((bufferq->response.offset + len) < bufferq->response.body_len) {
        /* Not all the data was written, update the offset and
         * reschedule the write event. */
        bufferq->response.offset += len;
        event_add(&client->ev_write, NULL);
        return;
    }

    /* The data was completely written, remove the buffer from the
     * write queue. */
    TAILQ_REMOVE(&client->writeq, bufferq, entries);

    free(bufferq->request.buf);
    free(bufferq->response.buf);

    if (NULL != bufferq->request.json) {
        cJSON_Delete(bufferq->request.json);
    }
    if (NULL != bufferq->response.json) {
        cJSON_Delete(bufferq->response.json);
    }

    free(bufferq);
}

void
on_accept(int fd, short ev, void *arg)
{
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct client *client;

    /* Accept the new connection. */
    client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1) {
        warn("accept failed");
        return;
    }

    /* Set the client socket to non-blocking mode. */
    if (setnonblock(client_fd) < 0)
        warn("failed to set client socket non-blocking");

    /* We've accepted a new client, allocate a client object to
     * maintain the state of this client. */
    client = calloc(1, sizeof(*client));
    if (client == NULL)
        err(1, "malloc failed");

    /* Setup the read event, libevent will call on_read() whenever
     * the clients socket becomes read ready.  We also make the
     * read event persistent so we don't have to re-add after each
     * read. */
    event_set(&client->ev_read, client_fd, EV_READ|EV_PERSIST, on_read,
        client);

    /* Setting up the event does not activate, add the event so it
     * becomes active. */
    event_add(&client->ev_read, NULL);

    /* Create the write event, but don't add it until we have
     * something to write. */
    event_set(&client->ev_write, client_fd, EV_WRITE, on_write, client);

    /* Initialize the clients write queue. */
    TAILQ_INIT(&client->writeq);

    zlog_debug(g_zc, "Accepted connection from %s\n",
        inet_ntoa(client_addr.sin_addr));
}

