#!/bin/sh

writefile=$1
writestr=$2

if [ $# -ne 2 ]
then 
	echo Number of Args are not correct
	exit 1
fi

if [ -d $writefile ]
then
	mkdir -p $writefile
fi

touch $writefile 

echo $2 >> $1
