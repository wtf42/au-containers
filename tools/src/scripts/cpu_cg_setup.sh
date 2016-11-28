#!/bin/bash

CONT_PID=$1
QUOTA=$2
WD=$3

CGMOUNT=$WD/.cg_cpu
CGDIR=$CGMOUNT/aucont
CONT_CGDIR=$CGDIR/$CONT_PID
CG_TYPE=$(lssubsys -a | grep -w "cpu" || echo "cpu")

mkdir -p $CGMOUNT
mountpoint -q $CGMOUNT || sudo mount -t cgroup -o $CG_TYPE aucont_cpu_cg $CGMOUNT

sudo mkdir -p $CGDIR
sudo chown -R $(id -u):$(id -g) $CGDIR

mkdir -p $CONT_CGDIR
echo $CONT_PID > $CONT_CGDIR/tasks

NPROC=$(nproc)
CG_PERIOD=1000000
CG_QUOTA=$((CG_PERIOD * NPROC * QUOTA / 100))

echo $CG_PERIOD > $CONT_CGDIR/cpu.cfs_period_us
echo $CG_QUOTA > $CONT_CGDIR/cpu.cfs_quota_us
