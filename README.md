# event-json-rpc

基于 libevent,以二进制头+json为协议来实现的 rpc 框架.

# 参考和依赖

```
libevent
cJSON
zlog
libzdb
```

## 以此为基础
https://github.com/jasonish/libevent-examples

## json c 解析器
https://github.com/kbranigan/cJSON

## c loger
https://github.com/HardySimpson/zlog

## 数据库连接池
http://re2c.org/
https://github.com/mverbert/libzdb http://www.tildeslash.com/libzdb

# 命令号和错误码

[命令号/错误码](/handler.h)

# Q&A

## 为什么不用 protobuf 或 thrift ?

因为我认为现在互联网开发的数据交互, json 已经成为事实标准,所以采用 json,这样在架构上让它直接,简单,有效,还兼具可扩展性,使那些只要支持 socket 编程的语言都可以不借助 protobuf/thrift 的扩展就能轻松实现 RPC.

## 为什么用 libevent,而不直接用 epoll?

libevent 已经经过无数网络程序验证过了,无需再造轮子,而且我造的轮子大概率没有现成的好.另外,我大部分程序是在 mac 下开发,跨平台的网络库比效率更重要.

# TODO

```
 _____ ___  ____   ___
|_   _/ _ \|  _ \ / _ \
  | || | | | | | | | | |
  | || |_| | |_| | |_| |
  |_| \___/|____/ \___/
```

[TODO](/TODO.md)
