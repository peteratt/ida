#!/bin/bash
#Start the servers

#Input the number of servers
#Output #creation of a file with the pid of servers

if `echo $1 | grep -q [^[:digit:]]`; then
   echo "$1 is not a number"
   exit 1
fi

#Filing neighbor config file for ZHT
ZHTNEIGHBORFILE=zhtNeighborFile
>$ZHTNEIGHBORFILE

PIDFILE=servers.pid
if [ -a $PIDFILE ]; then
	./stopServers.sh
	rm $PIDFILE
fi

>$PIDFILE

for ((i = 1; i <= $1; i++)); do
	port=$(($i+50000))
	echo "localhost:$port" >> $ZHTNEIGHBORFILE
done

FFSNETSERVER=../bin/bin/ffsnetd
ZHTSERVER=../lib/ZHT/bin/server_zht
ZHTCONFIG=../lib/ZHT/zht.cfg

TESTINGFOLDER=../testingEnv
BACKFROMTESTING=../../src

mkdir $TESTINGFOLDER
rm -rf $TESTINGFOLDER/*

for ((i = 1; i <= $1; i++)); do
	mkdir $TESTINGFOLDER/serv$1
	cd $TESTINGFOLDER/serv$1
	port=$(($i+9000))	
	$FFSNETSERVER $port &
	echo $! >> $PIDFILE
	cd $BACKFROMTESTING
	
	port=$(($i+50000))
	$ZHTSERVER $port $ZHTNEIGHBORFILE $ZHTCONFIG "UDP" &
	echo $! >> $PIDFILE
done

echo "............................................."
echo "Have fun now\n"