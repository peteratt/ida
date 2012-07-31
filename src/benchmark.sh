#!/bin/bash

k=$1
m=$2
bufsize=$3

./stopServers.sh
./startServers.sh 6

sleep 1

### SEND ###

cd ../examples

if [[ -f results_send_$k_$m_$bufsize.csv ]]; then
	rm results_send_$k_$m_$bufsize.csv
fi
touch results_send_$k_$m_$bufsize.csv

echo filename,filesize,thEncoding,thSending,thTotal >> results_send_$k_$m_$bufsize.csv #First line of the CSV

for file in ida/*
do
    if [[ -f $file ]]; then
    	echo $file
        ./fileSenderTEST $file $k $m $bufsize >> results_send_$k_$m_$bufsize.csv
    fi
done

### RECEIVE ###

if [[ -f results_recv_$k_$m_$bufsize.csv ]]; then
	rm results_recv_$k_$m_$bufsize.csv
fi

cd ../examples
touch results_recv_$k_$m_$bufsize.csv

echo filename,filesize,thEncoding,thSending,thTotal >> results_recv_$k_$m_$bufsize.csv #First line of the CSV

for file in ida/*
do
    if [[ -f $file ]]; then
        echo $file
        #./fileReceiverTEST $file >> results_recv_$k_$m_$bufsize.csv
    fi
done
