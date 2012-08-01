#!/bin/bash

k=$1
m=$2
bufsize=$3

./stopServers.sh
./startServers.sh 6

sleep 1

### SEND ###

cd ../examples

if [[ -f results_send.$k.$m.$bufsize.csv ]]; then
	rm results_send.$k.$m.$bufsize.csv
fi
touch results_send.$k.$m.$bufsize.csv

echo filename,filesize,thEncoding,thSending,thTotal >> results_send.$k.$m.$bufsize.csv #First line of the CSV

for file in ida/*
do
    if [[ -f $file ]]; then
    	echo $file
        ./fileSenderTEST $file $k $m $bufsize >> results_send.$k.$m.$bufsize.csv
    fi
done

### RECEIVE ###

if [[ -f results_recv.$k.$m.$bufsize.csv ]]; then
	rm results_recv.$k.$m.$bufsize.csv
fi

cd ../examples
touch results_recv.$k.$m.$bufsize.csv

echo filename,filesize,thEncoding,thSending,thTotal >> results_recv.$k.$m.$bufsize.csv #First line of the CSV

for file in ida/*
do
    if [[ -f $file ]]; then
        echo $file
        #./fileReceiverTEST $file >> results_recv.$k.$m.$bufsize.csv
    fi
done
