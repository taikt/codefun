#!/bin/sh

echo "Weillcom"
echo "con khi gio"

i=0
while [ $i -ne 40 ]
do
	i=$(($i+1))
	#echo "run slld $i"
	sldd_nadp nadp isAvailability &
	#echo "run slld $i done"
done
