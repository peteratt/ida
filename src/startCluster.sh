#!/bin/bash

hostsFile=$1
numServ=$1
rm zhtNeighborFile
>zhtNeighborFile

./fillNeighbors.sh $hostsFile 40000

#pssh -i -A -h pssh_hosts.txt -l palvare3 /export/home/palvare3/ida/src/fillNeighbors.sh 40000
pssh -i -A -h $hostsFile -l palvare3 /export/home/palvare3/ida/src/startServers.sh

#for ((i = 1; i <= $numServ; i++)); do
#	ssh palvare3@hec-$i 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/export/home/palvare3/ida/lib/udt4/src:/export/home/palvare3/protobuf/lib; nohup /export/home/palvare3/ida/bin/ffsnetd 49000 &'
#done
