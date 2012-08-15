#!/bin/bash
#Stop the servers

PIDFILE=/mnt/ssd/servers.pid
TESTINGFOLDER=/mnt/ssd/idaData

if [ -d "$TESTINGFOLDER" ]; then
	rm -rf $TESTINGFOLDER
fi

if [ ! -f $PIDFILE ]; then
	echo "$PIDFILE does not exist, exiting..."
	exit 1
fi

while read line; do
    kill $line
done < $PIDFILE



rm $PIDFILE
echo "............................................."
echo "Have fun now\n"
