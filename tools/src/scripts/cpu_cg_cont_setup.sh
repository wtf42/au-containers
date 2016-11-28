#!/bin/bash

CONT_PID=$1
EXEC_PID=$2
WD=$3

CGMOUNT=$WD/.cg_cpu
CGDIR=$CGMOUNT/aucont
CONT_CGDIR=$CGDIR/$CONT_PID

#mkdir -p $CGMOUNT
#sudo mount -t cgroup -o cpu,cpuacct aucont_cpu_cg $CGMOUNT

if [ -e $CONT_CGDIR ]; then
	echo $EXEC_PID >> $CONT_CGDIR/tasks
fi

#sudo umount $CGMOUNT
#rmdir $CGMOUNT
