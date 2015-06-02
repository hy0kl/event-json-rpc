#!/usr/bin/env php
<?php
/**
 * @describe:
 * @author: Jerry Yang(hy0kle@gmail.com)
 * */
error_reporting(E_ALL);
set_time_limit(0);
echo "<h2>TCP/IP Connection</h2>\n";

$port = 5555;
$ip = "127.0.0.1";

 /*
  +-------------------------------
  *    @socket连接整个过程
  +-------------------------------
  *    @socket_create
  *    @socket_connect
  *    @socket_write
  *    @socket_read
  *    @socket_close
  +--------------------------------
  */

$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
if (! $socket) {
    echo "socket_create() failed: reason: " . socket_strerror($socket) . "\n";
    exit;
}else {
    echo "socket_create() OK.\n";
}

echo "试图连接 '$ip' 端口 '$port'...\n";
$result = socket_connect($socket, $ip, $port);
if (! $result) {
    $err = socket_last_error($socket);
    echo "socket_connect() failed.\nReason: ($err) " . socket_strerror($err) . "\n";
    exit;
}else {
    echo "连接OK\n";
}

$input = array(
    'cmd'  => 1221,
    'data' => array(
        'test' => 'abc',
    ),
);
$input_json = json_encode($input);
$body_len   = strlen($input_json);

if (! socket_write($socket, $in, strlen($in))) {
    echo "socket_write() failed: reason: " . socket_strerror($socket) . "\n";
} else {
    echo "发送到服务器信息成功！\n";
    echo "发送的内容为:<font color='red'>$in</font>" . PHP_EOL;
}

echo '---------------' . PHP_EOL;

$buf = socket_read($socket, 4);
$body_len = array_merge(unpack('i', $buf));
echo print_r($body_len, true) . PHP_EOL;

$buf = socket_read($socket, 1024);
echo $buf . PHP_EOL;

echo '---------------' . PHP_EOL;

// while($out = socket_read($socket, 8192)) {
//     echo "接收服务器回传信息成功！\n";
//     echo "接受的内容为:",$out;
// }


echo "关闭SOCKET...\n";
socket_close($socket);
echo "关闭OK\n";

/* vi:set ts=4 sw=4 et fdm=marker: */

