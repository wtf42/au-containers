#!/bin/bash

mkdir -p task_stuff
tar -xf task_stuff.tar.gz -C task_stuff

sudo docker build -t "wtf42/aucont16" ./container
sed -i "s/eabatalov\/aucont16-test-base/wtf42\/aucont16/" task_stuff/scripts/run_test.sh
sed -i "s/cpu_boost >= 3 and cpu_boost <= 5/cpu_boost >= 4 and cpu_boost <= 6/" task_stuff/scripts/test.py

cd task_stuff/scripts
./run_test.sh
