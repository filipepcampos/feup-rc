#!/bin/bash

ip addr flush eth0
ip addr flush eth1
ip addr add 172.16.20.254/24 dev eth0
ip addr add 172.16.21.253/24 dev eth1
ip route add default via 172.16.21.254
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts