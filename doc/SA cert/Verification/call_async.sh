#!/bin/sh


i=0
while [ $i -ne 40 ]
do
	i=$(($i+1))
	#echo "run slld $i"
	sldd_nadp nadp isAvailabilityAsync &
	#echo "run slld $i done"
done
