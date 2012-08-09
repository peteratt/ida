#!/bin/bash
#Start the servers
#Input the number of servers
#Output #creation of a file with the pid of servers

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/export/home/palvare3/ida/lib/udt4/src:/export/home/palvare3/protobuf/lib
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
ZHTNEIGHBORFILE=/export/home/palvare3/ida/src/zhtNeighborFile

PIDFILE=/mnt/ssd/servers.pid
if [ -a $PIDFILE ]; then
	/export/home/palvare3/ida/src/stopServers.sh
fi

>$PIDFILE

#for ((i = 0; i < $NUMSERVERS; i++)); do
#	port=$(($i+40000))
#	echo `hostname` $port
#	echo `hostname` $port >> $ZHTNEIGHBORFILE
#done

FFSNETSERVER=/export/home/palvare3/ida/bin/ffsnetd
ZHTSERVER=/export/home/palvare3/ida/lib/ZHT/bin/server_zht
ZHTCONFIG=/export/home/palvare3/ida/lib/ZHT/zht.cfg

TESTINGFOLDER=/mnt/ssd/idaData
BACKFROMTESTING=/export/home/palvare3/ida/src/

mkdir $TESTINGFOLDER

for ((i = 0; i < $NUMSERVERS; i++)); do
	mkdir $TESTINGFOLDER/serv$i
	cd $TESTINGFOLDER/serv$i
	port=$(($i+49000))	
	$FFSNETSERVER $port &
	#echo `ps aux | grep palvare3`
	echo $! >> $PIDFILE
	cd $BACKFROMTESTING
	
	port=$(($i+40000))
	$ZHTSERVER $port $ZHTNEIGHBORFILE $ZHTCONFIG "UDP" &
	#echo `ps aux | grep palvare3`
	echo $! >> $PIDFILE
done

echo "............................................."
echo "Have fun now\n"
