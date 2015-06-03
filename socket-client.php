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
    echo "socket_create() failed: reason: " . socket_strerror($socket) . PHP_EOL;
    exit;
}else {
    echo "socket_create() OK.\n";
}

echo "试图连接 '$ip' 端口 '$port'...\n";
$result = socket_connect($socket, $ip, $port);
if (! $result) {
    $err = socket_last_error($socket);
    echo "socket_connect() failed.\nReason: ($err) " . socket_strerror($err) . PHP_EOL;
    exit;
}else {
    echo "连接OK\n";
}

$input = array(
    'cmd'  => 1221,
    'data' => array(
        'test' => 'abc',
        'mt_rand' => mt_rand(),
        'time'    => time(),
    ),
);
$input_json = json_encode($input);
$body_len   = strlen($input_json);
$req = pack('iA', $body_len, $input_json);

if (! socket_write($socket, $req, strlen($req))) {
    $err = socket_last_error($socket);
    echo "socket_write() failed: reason: " . socket_strerror($err) . PHP_EOL;
} else {
    echo "发送到服务器信息成功！[req: {$input_json}]" . PHP_EOL;
}

echo '---------------' . PHP_EOL;

$buf = socket_read($socket, 4);
$body_data = array_merge(unpack('i', $buf));
$body_len  = $body_data[0];

if ($body_len > 0) {
    echo sprintf('body_len: %d', $body_len) . PHP_EOL;
    $buf = socket_read($socket, $body_len);
    echo $buf . PHP_EOL;
}

echo '---------------' . PHP_EOL;

// while($out = socket_read($socket, 8192)) {
//     echo "接收服务器回传信息成功！\n";
//     echo "接受的内容为:",$out;
// }


echo "关闭SOCKET...\n";
socket_close($socket);
echo "关闭OK\n";

/* vi:set ts=4 sw=4 et fdm=marker: */

