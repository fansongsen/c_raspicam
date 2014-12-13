#!/bin/bash

counter=0
while true; do
		counter=$((counter+1))
		./test_movement_detection -f $1-$counter.h264 -d $2
	done
