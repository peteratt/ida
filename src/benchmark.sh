#!/bin/bash

k=$1
m=$2
bufsize=$3
targetDir=/mnt/ssd/idaTestFiles
resFileSend=results_send_`hostname`.$k.$m.$bufsize.csv
resFileRecv=results_recv_`hostname`.$k.$m.$bufsize.csv

sleep 1

### SEND ###

cd ../examples

if [[ -f $resFileSend ]]; then
	rm $resFileSend
fi
touch $resFileSend

echo filename,filesize,thEncoding,thSending,thTotal >> $resFileSend #First line of the CSV

for file in $targetDir/*
do
    if [[ -f $file ]]; then
    	echo $file
        ./fileSenderTEST $file $k $m $bufsize >> $resFileSend
    fi
done

### RECEIVE ###

if [[ -f $resFileRecv ]]; then
	rm $resFileRecv
fi

touch $resFileRecv

echo filename,filesize,thEncoding,thSending,thTotal >> $resFileRecv #First line of the CSV

for file in $targetDir/*
do
    if [[ -f $file ]]; then
        echo $file
        #./fileReceiverTEST $file >> results_recv.$k.$m.$bufsize.csv
    fi
done
