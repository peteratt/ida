#!/bin/bash
#Stop the servers

PIDFILE=/mnt/ssd/servers.pid
TESTINGFOLDER=/mnt/ssd/idaData

if [ -d "$TESTINGFOLDER" ]; then
	while true; do
		read -p "Do you wish to erase the entire folder $TESTINGFOLDER (this is irreversible)?" yn
		case $yn in
			[Yy]* ) rm -rf $TESTINGFOLDER; break;;
			[Nn]* ) break;;
			* ) echo "Please answer yes or no.";;
		esac
	done
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
