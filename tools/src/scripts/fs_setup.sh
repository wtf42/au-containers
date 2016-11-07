#!/bin/bash

root=$1

mount -t proc proc $root/proc
mount -t sysfs sys $root/sys
mount -t tmpfs none $root/dev

mkdir -p $root/dev/shm
mkdir -p $root/dev/mqueue
mount -t tmpfs none $root/dev/shm
mount -t mqueue none $root/dev/mqueue

[ -e $root/dev/null ] || touch $root/dev/null
mount --bind /dev/null $root/dev/null
[ -e $root/dev/zero ] || touch $root/dev/zero
mount --bind /dev/zero $root/dev/zero
[ -e $root/dev/urandom ] || touch $root/dev/urandom
mount --bind /dev/urandom $root/dev/urandom

mount --make-rprivate /
mkdir -m 0777 -p $root/old_root
mount --rbind $root/ $root/
pivot_root $root/ $root/old_root
umount -l /old_root
