#!/bin/bash

exit 0

root=$1

[ -e $root/dev/null ] || mknod -m 666 $root/dev/null c 1 3
[ -e $root/dev/zero ] || mknod -m 666 $root/dev/zero c 1 5
[ -e $root/dev/urandom ] || mknod -m 444 $root/dev/urandom c 1 9

#mknod -m 622 $root/dev/console c 5 1
#mknod -m 666 $root/dev/ptmx c 5 2
#mknod -m 666 $root/dev/tty c 5 0
#mknod -m 444 $root/dev/random c 1 8

chown 1000:1000 $root/dev/*
