#!/bin/sh

filedir=$1
searchstr=$2


if [ $# -ne 2 ]
then
	echo Wrong number of args
	exit 1
fi

#if [ -d $filedir ]
#then
	#echo Dir Does not exist
	#exit 1
#fi

matchedLines=$(grep -rl $2 $1 | wc -l)
#fileNum=$(ls -p | grep -v / | wc -l)

fileNum=$(find $1 -type f | wc -l)

echo The number of files are $fileNum and the number of matching lines are $matchedLines

