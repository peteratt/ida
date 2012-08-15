#!/bin/bash

hostsFile=$1
#numServ=$1
pssh -i -A -h $hostsFile /export/home/palvare3/ida/src/stopServers.sh

#for ((i = 1; i <= $numServ; i++)); do
#	ssh palvare3@hec-$i '/export/home/palvare3/ida/src/stopServers.sh &'
#done

