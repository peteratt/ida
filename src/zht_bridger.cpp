/**
 * File: ffsnet_bridger.cpp
 * Desc: This is a C wrapper to call the ffsnet library
 * Author: dzhao8@hawk.iit.edu
 * History:
 * 		06/25/2011 - initial development
 *
 * Compile to a shared library:
 *		g++ -fPIC ffsnet_bridger.cpp --shared -o libffsnet_bridger.so -L. -lffsnet
 *
 */

#include "../inc/zht_bridger.h"
#include "../lib/ZHT/inc/meta.pb.h"
#include "../lib/ZHT/inc/cpp_zhtclient.h"

const char* zht_parse_meta(struct metadata* meta) {
	Package package;
	package.set_virtualpath(meta->filename); //as key
	package.set_isdir(false);
	package.set_replicano(1);
	package.set_operation(3); //1 for look up, 2 for remove, 3 for insert
	package.set_realfullpath(meta->filename);
	
	// IDA metadata
	package.set_k(meta->k);
	package.set_m(meta->m);
	package.set_encodinglib(meta->encodingLib);
	package.set_filesize(meta->fileSize);
	package.set_bufsize(meta->bufsize);
	
	std::string serialized = package.SerializeAsString();
	
	/*
	Package package2;
	package2.ParseFromString(serialized);
	
	int k = package2.k();
	*/

	return serialized.c_str();
}


template<class bidiiter>
bidiiter random_unique(bidiiter begin, bidiiter end, size_t num_random) {
    size_t left = std::distance(begin, end);
    while (num_random--) {
        bidiiter r = begin;
        std::advance(r, rand()%left);
        std::swap(*begin, *r);
        ++begin;
        --left;
    }
    return begin;
}

int ZHTgetLocations(ZHTClient_c zhtClient, struct comLocations * loc, int n){
	//This makes a connection to ZHT to get the memberlist and picks n locations to store our files.
	ZHTClient * zhtcppClient = (ZHTClient *) zhtClient;

	vector<struct HostEntity> potentialLoc(zhtcppClient->memberList); //we need to pick here :) Maybe need to include zht_util to decrypt HostEntity

	if(potentialLoc.size() < n) return 1; //should be NOT ENOUGH LOCATIONS ERROR
	
	//This for allows to pick n random different members in the vector ---------
	//Laziness: http://ideone.com/3A3cv and http://stackoverflow.com/questions/9345087/choose-m-elements-randomly-from-a-vector-containing-n-elements
	random_unique(potentialLoc.begin(), potentialLoc.end(), n);
    
    for(int i=0; i<n; ++i) {
        //std::cout << a[i] << '\n';
        
        /*
        current = (struct comTransfer *) malloc(sizeof(struct comTransfer));
	
		current->hostName = (char *) malloc(strlen("localhost")+1);
		strcpy(current->hostName,"localhost");
		
		current->port = FFSNETPORTSTART + i;
		
		sprintf(chunkname, "%s.%d", filehash, i);
		current->distantChunkName = (char *) malloc(strlen(chunkname)+1);
		strcpy(current->distantChunkName,chunkname);
		
		current->localChunkName = (char *) malloc(strlen(chunkname)+1);
		strcpy(current->localChunkName,chunkname);
		
		currentLocationsNumber++;
		current->next = prev;
		prev = current;
		*/
        
        
    }
	//---------------------

	return 0;
}
