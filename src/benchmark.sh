#!/bin/bash

### SEND ###

if [[ -f results_send.csv ]]; then
	rm results_send.csv
fi

cd ../examples
touch results_send.csv

echo filename,filesize,thEncoding,thSending,thTotal >> results_send.csv #First line of the CSV

for file in ./ida/*
do
    if [[ -f $file ]]; then
        ./fileSenderTEST $file $k $m $bufsize >> results_send.csv
    fi
done

### RECEIVE ###

if [[ -f results_recv.csv ]]; then
	rm results_recv.csv
fi

cd ../examples
touch results_recv.csv

echo filename,filesize,thEncoding,thSending,thTotal >> results_recv.csv #First line of the CSV

for file in ./ida/*
do
    if [[ -f $file ]]; then
        ./fileReceiverTEST $file >> results_recv.csv
    fi
done