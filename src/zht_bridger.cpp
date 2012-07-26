/**
 * File: zht_bridger.cpp
 * Desc: This is a C wrapper to call the ZHT Protocol Buffers library
 * Author: palvare3@hawk.iit.edu
 * Author: cdebains@hawk.iit.edu
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
	
	return serialized.c_str();
}

struct metadata* zht_unparse_meta(const char* text) {

	Package package;
	package.ParseFromString(text);
	
	struct metadata* meta;
	
	meta->filename = package.virtualpath().c_str();
	meta->k = package.k();
	meta->m = package.m();
	meta->bufsize = package.bufsize();
	meta->fileSize = package.filesize();
	meta->encodingLib = package.encodinglib();
	
	return meta;
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

int ZHTgetLocations(ZHTClient_c zhtClient, struct comLocations * loc){
	//This makes a connection to ZHT to get the memberlist and picks blocksNumber locations to store our files.
	ZHTClient * zhtcppClient = (ZHTClient *) zhtClient;

	int blocksNumber = loc->locationsNumber;
	
	vector<struct HostEntity> potentialLoc(zhtcppClient->memberList); //we need to pick here :) Maybe need to include zht_util to decrypt HostEntity

	if(potentialLoc.size() < blocksNumber){
		loc->locationsNumber = potentialLoc.size();
	}
	
	//This for allows to pick n random different members in the vector ---------
	//Laziness: http://ideone.com/3A3cv and http://stackoverflow.com/questions/9345087/choose-m-elements-randomly-from-a-vector-containing-n-elements
	random_unique(potentialLoc.begin(), potentialLoc.end(), blocksNumber);
    
    
	int currentLocationsNumber = 0;
	int i;
	
	int FFSNETSHIFT = 9000;
	
	struct comTransfer * prev = NULL;
	struct comTransfer * current;
	
	
    for(int i=0; i<blocksNumber; ++i) {
        //std::cout << a[i] << '\n';
              
        current = (struct comTransfer *) malloc(sizeof(struct comTransfer));
	
		current->hostName = (char *) malloc(potentialLoc[i].host.size()+1);
		strcpy(current->hostName,potentialLoc[i].host.c_str());
		
		current->port = potentialLoc[i].port + FFSNETSHIFT;
		
	
		currentLocationsNumber++;
		current->next = prev;
		prev = current;

        
    }
	loc->transfers = current;

	return 0;
}
