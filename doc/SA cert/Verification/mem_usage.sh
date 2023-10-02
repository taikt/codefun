#!/bin/sh


id=$(pgrep nadProxy)
i=0
while [ $i -ne 20 ]
do
	i=$(($i+1))
	cat /proc/$id/status | grep VmRSS
	#echo $id
	sleep 1
done
