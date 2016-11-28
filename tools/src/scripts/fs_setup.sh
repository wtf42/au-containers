#!/bin/bash

ROOT=$1

mount -t proc proc $ROOT/proc
mount -t sysfs sys $ROOT/sys
mount -t tmpfs none $ROOT/dev

mkdir -p $ROOT/dev/shm
mkdir -p $ROOT/dev/mqueue
mount -t tmpfs none $ROOT/dev/shm
mount -t mqueue none $ROOT/dev/mqueue

[ -e $ROOT/dev/null ] || touch $ROOT/dev/null
mount --bind /dev/null $ROOT/dev/null
[ -e $ROOT/dev/zero ] || touch $ROOT/dev/zero
mount --bind /dev/zero $ROOT/dev/zero
[ -e $ROOT/dev/urandom ] || touch $ROOT/dev/urandom
mount --bind /dev/urandom $ROOT/dev/urandom

mount --make-rprivate /
mkdir -m 0777 -p $ROOT/old_root
mount --rbind $ROOT/ $ROOT/
pivot_root $ROOT/ $ROOT/old_root
umount -l /old_root
