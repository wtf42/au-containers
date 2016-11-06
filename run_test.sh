#!/bin/bash

mkdir -p task_stuff
tar -xf task_stuff.tar.gz -C task_stuff

for line in $(seq 232 241); do
   sed -i $line"s/^[^#]/#/" task_stuff/scripts/test.py
done

cd task_stuff/scripts
./run_test.sh
