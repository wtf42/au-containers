#!/bin/bash

PID=$1
IP_CONT=$2
IP_HOST=$3

HOST=veth_h$PID
CONT=veth_c$PID

sudo ip link add $HOST type veth peer name $CONT
sudo ip link set $CONT netns $PID
sudo ip link set $HOST up
sudo ip addr add $IP_HOST/24 dev $HOST
sudo sysctl -q net.ipv4.conf.all.forwarding=1
