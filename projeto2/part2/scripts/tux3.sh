#!/bin/bash

ip addr flush eth0
ip addr add 172.16.Y0.1/24 dev eth0
ip route add 172.16.Y1.0/24 via 172.16.Y0.254
ip route add default via 172.16.Y0.254