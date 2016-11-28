#!/bin/bash

CONT_PID=$1
EXEC_PID=$2
WD=$3

CGMOUNT=$WD/.cg_cpu
CGDIR=$CGMOUNT/aucont
CONT_CGDIR=$CGDIR/$CONT_PID

if [ -e $CONT_CGDIR ]; then
    echo $EXEC_PID >> $CONT_CGDIR/tasks
fi
