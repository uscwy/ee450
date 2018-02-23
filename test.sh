#!/bin/bash

if [ ! -f "./client" ] ; then
	echo "file not exist"
	exit 0
fi

for i in {1..100000..1}
do
	./client $RANDOM
done

