#!/bin/bash

mkdir -p task_stuff
tar -xf task_stuff.tar.gz -C task_stuff

cd task_stuff/scripts
./run_test.sh
