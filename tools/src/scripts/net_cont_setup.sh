#!/bin/bash

PID=$1
IP_CONT=$2
IP_HOST=$3

HOST=veth_h$PID
CONT=veth_c$PID

ip link set lo up
ip link set $CONT up
ip addr add $IP_CONT/24 dev $CONT
ip route add default via $IP_HOST
