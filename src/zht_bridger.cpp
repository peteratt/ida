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

//DEPRECATED
const char* zht_parse_meta(struct metadata* meta) {
	Package package;
	package.set_virtualpath(meta->key); //as key
	package.set_isdir(false);
	package.set_replicano(1);
	package.set_operation(3); //1 for look up, 2 for remove, 3 for insert
	package.set_realfullpath(meta->physicalPath);
	
	// IDA metadata
	package.set_k(meta->k);
	package.set_m(meta->m);
	package.set_encodinglib(meta->encodingLib);
	package.set_filesize(meta->fileSize);
	package.set_bufsize(meta->bufsize);
	
	std::string serialized = package.SerializeAsString();
	
	cout << serialized << endl;
	dbgprintf("Serialized:%s \n",serialized.c_str());
	dbgprintf("k:%i,m:%i,encodinglib:%i,filesize:%i \n",package.k(),package.m(),package.encodinglib(),package.filesize());
	
	const char * resString = serialized.c_str();
	
	Package package2;
	package2.ParseFromString(string(resString));
	dbgprintf("k:%i,m:%i,encodinglib:%i,filesize:%i \n",package2.k(),package2.m(),package2.encodinglib(),package2.filesize());


	std::string reborn = string(resString);

	Package package3;
	package3.ParseFromString(reborn);
	dbgprintf("k:%i,m:%i,encodinglib:%i,filesize:%i \n",package3.k(),package3.m(),package3.encodinglib(),package3.filesize());

	
	
	
	return resString;
}

int zht_insert_meta(ZHTClient_c zhtClient, struct metadata * meta){

	ZHTClient * zhtcppClient = (ZHTClient *) zhtClient;

	Package package;
	string keyStr(meta->key);
	
	//1. General infos on the file/storage
	package.set_virtualpath(keyStr); //as key
	package.set_isdir(false);
	package.set_replicano(1);
	package.set_operation(3); //1 for look up, 2 for remove, 3 for insert
	package.set_realfullpath(keyStr);
	
	//2. General infos on the encoding
	package.set_k(meta->k);
	package.set_m(meta->m);
	package.set_encodinglib(meta->encodingLib);
	package.set_filesize(meta->fileSize);
	package.set_bufsize(meta->bufsize);
	
	//3. Chunks/Replicas locations
	struct comTransfer * current = meta->loc->transfers;
	Package_Location*  location;

	while(current != NULL){
		location = package.add_location();
		
		location->set_hostname(string(current->hostName));
		location->set_port(current->port);
		location->set_distantchunkname(string(current->distantChunkName));
		
		current=current->next;
	}
	//4. Insertion
	std::string serialized = package.SerializeAsString();
	
	dbgprintf("Package Length:%i\n",serialized.size());
	
	//TEST
	Package package4;
	package4.ParseFromString(serialized);
	dbgprintf("Serialized: %s \n",serialized.c_str());
	dbgprintf("key: %s \n",(package4.virtualpath()).c_str());
	dbgprintf("k:%i,m:%i,encodinglib:%i,filesize:%i \n",package4.k(),package4.m(),package4.encodinglib(),package4.filesize());
	
	for (int j = 0; j < package4.location_size(); j++) {
		const Package_Location& location = package4.location(j);
		
		dbgprintf("chunk:%s, port:%i\n",location.distantchunkname().c_str(), location.port());
	}	
	//END TEST
	
	
	int res = zhtcppClient->insert(serialized);
	
	return res;
}


//DEPRECATED
struct metadata* zht_unparse_meta(const char* text) {

	Package package;
	package.ParseFromString(text);
	
	struct metadata* meta = (struct metadata*)malloc(sizeof(struct metadata));
	meta->loc = (struct comLocations *) malloc(sizeof(struct comLocations));
	
	meta->key = package.virtualpath().c_str();
	meta->k = package.k();
	meta->m = package.m();
	meta->bufsize = package.bufsize();
	meta->fileSize = package.filesize();
	meta->encodingLib = package.encodinglib();
	
	return meta;
}

struct metadata * zht_lookup_meta(ZHTClient_c zhtClient, const char * key){

	//1. Lookup in ZHT
	ZHTClient * zhtcppClient = (ZHTClient *) zhtClient;

	string keyStr(key);
	string resSerializedPackage;
	
	Package keyPackage;
	keyPackage.set_virtualpath(keyStr); //as key
	keyPackage.set_isdir(false);
	keyPackage.set_replicano(1);
	keyPackage.set_operation(1); //1 for look up, 2 for remove, 3 for insert
	
	int res = zhtcppClient->lookup(keyPackage.SerializeAsString(),resSerializedPackage);

	dbgprintf("Package Length:%i\n",resSerializedPackage.size());

	
	//2. Parse Package and fill meta
	Package package;
	package.ParseFromString(resSerializedPackage);
	
	struct metadata* meta = (struct metadata*)malloc(sizeof(struct metadata));
	
	//2.1 General file infos
	meta->key = package.virtualpath().c_str();
	meta->k = package.k();
	meta->m = package.m();
	meta->bufsize = package.bufsize();
	meta->fileSize = package.filesize();
	meta->encodingLib = package.encodinglib();
	
	//2.2 Locations
	meta->loc = (struct comLocations *) malloc(sizeof(struct comLocations));

	struct comTransfer * current;
	struct comTransfer * prev = NULL;

	for (int j = 0; j < package.location_size(); j++) {
		current = (struct comTransfer *) malloc(sizeof(struct comTransfer));
	
		const Package_Location& location = package.location(j);
	
		//const std::string host = location.hostname();
		current->hostName = (char *) malloc((location.hostname()).size()+1);
		strcpy(current->hostName,location.hostname().c_str());
		
		current->port = location.port();
		
		current->distantChunkName = (char *) malloc((location.distantchunkname()).size()+1);
		strcpy(current->distantChunkName,location.distantchunkname().c_str());
		
		dbgprintf("Host (%i):%s - port: %i - chunkname: %s\n",j,location.hostname().c_str(),location.port(), location.distantchunkname().c_str());
		
		current->next = prev;
		prev = current;
	}
	meta->loc->transfers = current;
	
	
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
	
	//This allows to pick n random different members in the vector ---------
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
