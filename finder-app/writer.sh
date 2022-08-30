#!/bin/sh

# References : https://linuxtect.com/recursive-mkdir-command-in-linux/#:~:text=The%20%2Dp%20option%20is%20used,of%20them%20are%20created%20automatically 

#check if there are two arguments 
if [ $# -ne 2 ]
then 
	echo "Wrong number of args. Use 2 arguments"
	exit 1
fi

writefile=$1
writestr=$2

#get the directory from full path which includes file name also
filedir=$(dirname $writefile)


#checks is directory exist. if not creates the directory 
if [ ! -d $filedir ]
then
	mkdir -p $(dirname $filedir)
	
	#checks if directory was made successfully otherwise exits with 1
	if [ $? ]
	then 
		echo "Failed to create directory"
		exit 1
	fi
fi

#overrides/replaces previous data in file
echo $writestr > $writefile

# checks if string was written into file
if [ $? ]
then 
	echo "Failed to write into file"
	exit 1
fi
