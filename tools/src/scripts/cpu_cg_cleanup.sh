#!/bin/bash

WD=$1

CGMOUNT=$WD/.cg_cpu

if [ -e $CGMOUNT ]; then
    sudo umount $CGMOUNT
    rmdir $CGMOUNT
fi
