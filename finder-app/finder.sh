#!/bin/sh
#R eferences: 1. https://www.cyberciti.biz/faq/grep-count-lines-if-a-string-word-matches/
# 2. https://stackoverflow.com/questions/8509226/using-find-command-in-bash-script


# check if there are two arguments 
if [ $# -ne 2 ]
then
	echo "Wrong number of args. Use 2 arguments"
	exit 1
fi

# First argument is file dir path and 2nd argument is the string used to compare for matches 
filedir=$1
searchstr=$2

# check is entered directory is a valid path
if [ ! -d $filedir ]
then
	echo "Directory Does not exist. Enter valid directory"
	exit 1
fi

# get the number of matched lines
matchedLines=$(grep -rl $2 $1 | wc -l)

#f ind the total number of files within a directory and subdirectories
fileNum=$(find $1 -type f | wc -l)

echo "The number of files are $fileNum and the number of matching lines are $matchedLines"

