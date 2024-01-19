#!/bin/bash

procs=$(ps | grep gdbserver | grep -v $(basename $0) | grep -v grep)
echo "$procs"

ids=$(echo "$procs" | grep -o -E "[0-9]+\ .*\ [0-9]+" | grep -o -E "[0-9]+\ ")
for id in $ids
do
	echo "kill -9 $id"
	kill -9 $id
done

gdbserver localhost:2000 "$@"
