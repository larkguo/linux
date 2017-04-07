#!/bin/sh

gcc client.c -o client

chmod 777 *

killall -9 client

./client -i tun134 -c 192.168.109.132 &

ifconfig tun134 192.168.8.134

ip addr add 192.168.8.0/24 dev tun134

ping 192.168.8.132 -I 192.168.8.134 -c 1


