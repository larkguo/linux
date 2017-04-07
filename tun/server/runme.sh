#!/bin/sh

gcc server.c -o server

chmod 777 *

killall -9 server


./server -i tun132 &

ifconfig tun132 192.168.8.132

ip addr add 192.168.8.0/24 dev tun132


