#!/bin/sh

maxNumber=0
maxID="Error! No input was given!"
curNumber=0
curID=0

for studentID in `sort`
do
	if [ "$studentID" = "$curID" ]
	then
		curNumber=$(expr $curNumber + 1)
	else
		if [ $curNumber -gt $maxNumber ]
		then
			maxNumber=$curNumber
			maxID="$curID"
		fi

		curID="$studentID"
		curNumber=1
	fi
done

if [ $curNumber -gt $maxNumber ]
then
	maxNumber=$curNumber
	maxID="$curID"
fi

echo "$maxID"
