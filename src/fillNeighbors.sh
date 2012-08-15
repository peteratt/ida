ZHTNEIGHBORFILE=/export/home/palvare3/ida/src/zhtNeighborFile
hostsFile=$1
port=$2
#echo `hostname` $port >> $ZHTNEIGHBORFILE
for i in `cat $hostsFile` ; do echo $i $port >> $ZHTNEIGHBORFILE; done
