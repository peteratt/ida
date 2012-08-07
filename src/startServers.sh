#!/bin/bash
#Start the servers
#Input the number of servers
#Output #creation of a file with the pid of servers


NUMSERVERS=1 #Default number of servers is only one per node

if `echo $1 | grep -q [^[:digit:]]`; then
   echo "$1 is not a number"
   exit 1
fi

if [ -z "$1" ]; then
	echo "Starting $NUMSERVERS servers"
else
	NUMSERVERS=$1
	echo "Starting $NUMSERVERS servers"
fi




#Filing neighbor config file for ZHT
ZHTNEIGHBORFILE=zhtNeighborFile
>$ZHTNEIGHBORFILE

PIDFILE=/mnt/ssd/servers.pid
if [ -a $PIDFILE ]; then
	./stopServers.sh
	rm $PIDFILE
fi

>$PIDFILE

for ((i = 0; i < $NUMSERVERS; i++)); do
	port=$(($i+40000))
	echo "localhost $port" >> $ZHTNEIGHBORFILE
done

FFSNETSERVER=~/ida/bin/ffsnetd
ZHTSERVER=~/ida/lib/ZHT/bin/server_zht
ZHTCONFIG=~/ida/lib/ZHT/zht.cfg

TESTINGFOLDER=/mnt/ssd/idaData
BACKFROMTESTING=~/ida/src/

mkdir $TESTINGFOLDER

for ((i = 0; i < $NUMSERVERS; i++)); do
	mkdir $TESTINGFOLDER/serv$i
	cd $TESTINGFOLDER/serv$i
	port=$(($i+49000))	
	$FFSNETSERVER $port &
	echo $! >> $PIDFILE
	cd $BACKFROMTESTING
	
	port=$(($i+40000))
	$ZHTSERVER $port $ZHTNEIGHBORFILE $ZHTCONFIG "UDP" &
	echo $! >> $PIDFILE
done

echo "............................................."
echo "Have fun now\n"
