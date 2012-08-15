/*
 * Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * Author:  Pedro Alvarez-Tabio
 * Email:   palvare3@iit.edu
 *
 */


#include   <stdbool.h>

#include "../inc/ida_util.h"
#include "../inc/zht_bridger.h"
#include "../inc/ec.h"
#include "../inc/ffsnet.h"

//GPU vs NOGPU -------
#ifndef IDA_NOGPU
	#include "../inc/cudaCheck.h"
	#define CUDA_CHECK() CUDACapableCheck()
	#define EC_GPULIB_INIT(x) ec_init_Gibraltar(x)
#else
	#define CUDA_CHECK() 1
	#define EC_GPULIB_INIT(x) ec_init_Library(1, x)
#endif
//GPU vs NOGPU -------

#include <c_zhtclientStd.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>


ZHTClient_c zhtClient;
int GPU_CAPABLE;
const int LOOKUP_SIZE = 65535;

int ida_init(char* neighbors, char* config){

	//1. Initialize ZHT	
	c_zht_init_std(&zhtClient, neighbors, config, false); //neighbor zht.cfg false=UDP

	//2. Check GPU Capabilities
	GPU_CAPABLE = CUDA_CHECK();

	return 0;
}

int ida_finalize(){
	
	//1. Close ZHT
	c_zht_teardown_std(zhtClient);

	return 0;
}

int ecfillmeta(const char * key, const char * physicalPath, int k, int m , int encodingLib, int bufsize, struct metadata ** metaP){
	//This function fills the metadata for a coming insertion
	//meta should already be allocated (stack or heap).
	//OUTPUT filled meta :)
	
	// Allocate META (SHOULD BE FREE BY THE USER outside of this function)
	*metaP = (struct metadata*)malloc(sizeof(struct metadata));
	struct metadata * meta = *metaP;
	
	/* 1. Basic Parameters */
	//key (virtual Path)
	meta->key = key;
	
	//Physical Path (may be less relevant when downloading, will be the default physical path maybe?)
	meta->physicalPath = physicalPath;
	
	//k (for EC)
	meta->k = k;
	
	//m (for EC)
	meta->m = m;
	
	//encodingLib (for EC)
	meta->encodingLib = encodingLib;
	
	//bufsize (for EC)
	meta->bufsize = bufsize;
	
	//filesize
	FILE *source;
	source = fopen(physicalPath, "rb");
	if(!source){
		dbgprintf("Metadata Fill ERROR: %s\n", strerror(errno));
		exit(EXIT_FAILURE);//maybe a return 1 could be appropriate?
	}
	fseek(source, 0L, SEEK_END);
	meta->fileSize = ftell(source);
	fclose(source);

	/* 2. Locations */
	meta->loc = (struct comLocations *) malloc(sizeof(struct comLocations));
	meta->loc->locationsNumber = k+m;
	
	return getSendLocations(k, getchunksize(meta->fileSize, k, bufsize), meta->loc);
}


int ec_init_Library(int libraryId, ecFunctions *ec){

	//This function has to be modified for any library addition
	switch(libraryId){
		case GIBRALTAR:
			if(GPU_CAPABLE){
				EC_GPULIB_INIT(ec);
			}
			else{
				ec_init_JerasureRS(ec);
				return 2;//SHOULD BE DEFAULT LIBRARY ERROR
			}
			break;
		case JERASURERS:
			ec_init_JerasureRS(ec);
			break;
		default:
			return 1;
	}
	
	return 0;
}

int ecFileEncode(struct metadata * meta){
	
	//INPUTS: METADATA
	
	// v2 improvement here: we open N sockets using UDT, and send buffers just after encoding them (streaming style)
	
	int bufsize = meta->bufsize;
	int k = meta->k;
	int m = meta->m;
	int libraryId = meta->encodingLib;
	
	/* Initialize Ec Functions and Context */
	ecFunctions ec;
    ecContext context;

	ec_init_Library(libraryId, &ec);
	
	int rc = ec->init(k, m, &context);
	if (rc) {
		dbgprintf("Error:  %i\n", rc);
		exit(EXIT_FAILURE);
	}

	/* Allocate/define the appropriate number of buffers */
	void *buffers;
	ec->alloc(&buffers, bufsize, &bufsize, context);
	
	// NOTE: In case encoding/networking have too different speeds, a buffer storing encoding data could be useful to make sure we are not overflowing the send buffer (or we need to make sure that it stops if the buffer is full)
	// !!!!!NOTE2: to accelerate the reading process in the file, a bigger part of the file could be read at once to fill a buffer (CHECK setvbuf to modify fread buffer!!!)
	
	
	/* Open source file */
	FILE * source = fopen(meta->physicalPath, "rb");
	if(!source){
		dbgprintf("ERROR: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Open destination sockets */
	UDTArray_c socks = NULL; //NULL is here to remove (warning: ‘socks’ may be used uninitialized in this function)
	int * index = Transfer_init_c(&socks, meta, CLIENT_SENDBUF);
	if(*index == -1){
		printf("Socket Creation Error (not enough targets or too many errors). Aborting..\n");
		exit(EXIT_FAILURE);
	}
	
	/* Encode N*BUFSIZE bytes at a time then repeat until EOF */
	int bytesRead;
	int retSend=-1;
	int totalTrans;
	
	//int BytesTrans=0;
	while(!feof(source)){
		//1-read		
		bytesRead = fread(buffers,sizeof(unsigned char), bufsize*k, source);

		if(bytesRead < bufsize * k){//Padding(with 0s) if necessary
			memset((unsigned char *) buffers + bytesRead, 0, bufsize*k - bytesRead);
		}
		
		if(bytesRead > 0){
			//2-encode
			ec->generate(buffers, bufsize, context);
			//3-Write
			int j;
			for (j = 0; j < k + m; j++) {
					retSend = -1;
					totalTrans = 0;
					while(retSend == -1 || totalTrans != bufsize){
						retSend = bufferSend_c(socks, index[j], (unsigned char *)buffers + j*bufsize + totalTrans, bufsize - totalTrans);//Push data into the sending queue
						if(retSend != -1) totalTrans += retSend;
					}
					//BytesTrans +=totalTrans;
			}
			//dbgprintf("BytesTrans:%i\n",BytesTrans);
		}
	}

	/* Closing the sockets */
	Transfer_destroy_c(socks); 

	/* Free allocated memory and destroy Context */
	ec->free(buffers, context);
	ec->destroy(context);
	
	return 0;
}

int ecFileDecode(char *filepath, struct metadata * meta) {
	//INPUTS: Filename to Decode
	char filenameDest[260];	
	int bufsize,n,m,filesize,ngood,nbad,j,i,unfinished,nBytes,tmp, towrite, libraryId;

	filesize = meta->fileSize;
	n = meta->k;
	m = meta->m;
	bufsize = meta->bufsize;
	libraryId = meta->encodingLib;


	/* Initialize Ec Functions and Context */
	ecFunctions ec;
    ecContext context;

	ec_init_Library(libraryId, &ec);
	
	int rc = ec->init(n, m, &context);
	if (rc) {
		dbgprintf("Error:  %i\n", rc);
		exit(EXIT_FAILURE);
	}

	/* Allocate/define the appropriate number of buffers */
	void *buffers;
	ec->alloc(&buffers, bufsize, &bufsize, context);

	/* Open Reception Sockets */
	UDTArray_c socks = NULL; //NULL is here to remove (warning: ‘socks’ may be used uninitialized in this function)
	int * index = Transfer_init_c(&socks, meta, CLIENT_RECVBUF);
	if(*index == -1){
		printf("Socket Creation Error (not enough targets or too many errors). Aborting..\n");
		exit(EXIT_FAILURE);
	}
	
	/* Identify missing files and open files */
	//TODO Check files checksum + length to garantee correctness
	FILE *destination;

	int goodBufIds[m+n];
	int badBufIds[m+n];
	
	ngood=0;
	nbad=0;

	destination = fopen(filepath, "wb");
	if(!destination){
		dbgprintf("ERROR: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	int currentIndex=0;

	for (j = 0; j < n + m; j++) {
		if(index[j-nbad] != j){
			if(j < n) badBufIds[nbad++] = j; // There was a if(j < n) .....why?!, do we stop counting bad buffers after n is passed?
		}
		else{
			goodBufIds[ngood++] = j;
		}
	}

	/* Shuffling */
	for(j=0; j < ngood; j++){
		if(goodBufIds[j] != j && goodBufIds[j] < n){
			i = j+1;
			while(goodBufIds[i] < n) i++;
			tmp = goodBufIds[i];
			memmove(goodBufIds+j+1, goodBufIds+j, sizeof(int)*(i-j));
			goodBufIds[j] = tmp;
		}
	}
	
	unfinished = 1;
	nBytes = 0;
	int retSend=-1;
	int totalTrans;
	
	//int BytesTrans = 0;
	
	while (unfinished) {
		for (j = 0; j < ngood; j++) {
			retSend = -1;
			totalTrans = 0;

			while(retSend == -1 || totalTrans != bufsize){
				retSend = bufferRecv_c(socks, goodBufIds[j], buffers + j*bufsize + totalTrans, bufsize - totalTrans);//Push data into the sending queue
				if(retSend != -1) totalTrans += retSend;
			}
			//BytesTrans +=totalTrans;
			//dbgprintf("BytesTrans:%i\n",BytesTrans);
		}
	
		int buf_ids[256];
		memcpy(buf_ids, goodBufIds, n*sizeof(int));
		memcpy(buf_ids + n, badBufIds, nbad*sizeof(int));
		ec->recover(buffers, bufsize, buf_ids, nbad, context);
	
		/* Replace buffers */
		for (i = 0; i < n; i++) {
			if (buf_ids[i] != i) {
				j = i+1;
				while (buf_ids[j] != i) j++;
				memcpy(buffers + bufsize*i, buffers + bufsize*j, bufsize);
				buf_ids[i] = i;
			}
		}
		
		if (nBytes + n*bufsize >= filesize) {
			towrite = filesize - nBytes;
			unfinished = 0;
		} else {
			towrite = n * bufsize;
		}
 		
		nBytes += towrite;
	
		/* Write buffers to file */
		fwrite((unsigned char*)buffers, sizeof(unsigned char), towrite, destination);
	}
	
	/* Close files */
	fclose(destination);
	
	/* Closing the sockets */
	Transfer_destroy_c(socks); 
	
	/* Free allocated memory and destroy Context */
	ec->free(buffers, context);
    	ec->destroy(context);
	
	return EXIT_SUCCESS;
}


int getSendLocations(int minimum, int chunksize, struct comLocations * loc){
	// This functions fill the comLocations structure that contains future chunk locations.
	
	
	int blocksNumber = loc->locationsNumber; //blocksNumber is the number of actual data blocks available
	int i;
	
	char chunkname[CHUNKNAME_LENGTH];
	unsigned int chunknameLen = CHUNKNAME_LENGTH;
	//1. We acquire the locations in ZHT
	ZHTgetLocations(zhtClient, loc);
	
	struct comTransfer * current = loc->transfers;
	int LocationsNumber = loc->locationsNumber;

	if(LocationsNumber < minimum){
		return 1; //That should be defined as a constant: NOTENOUGHLOCATIONS
	}
	
	blocksNumber = LocationsNumber; //The blocksNumber is maxed by the available Locations number

	//2. We have the destination nodes, we need to attribute blocks to them.
	//2.1 RandomFilename
	char filehash[64];
	randomStr(filehash,64);
		
	
	for (i = 0; i < blocksNumber; i++) {	
		
		chunknameLen = sprintf(chunkname, "%s.%d", filehash, i);
		current->distantChunkName = (char *) malloc(chunknameLen+1);
		strcpy(current->distantChunkName,chunkname);
		
		current->chunkNumber = i;
		
		current = current->next;
	}
	
	loc->locationsNumber = blocksNumber; // We actually tell the program how many blocks we can transfer
	
	return 0;

}


int getRecvLocations(char * filename, struct comLocations * loc, int minimum){
	
	//TODO This function should check if the files are available.
	
	return 0;
}

void free_struct_comLocations(struct comLocations * loc){
	
	struct comTransfer * current = loc->transfers;
	struct comTransfer * tofree;
	
	while(current != NULL){
		free(current->hostName);
		free(current->distantChunkName);
		
		tofree = current;
		current = current->next;
		free(tofree);
	}
}

int ecInsertMetadata(struct metadata* meta) {
	
	return zht_insert_meta(zhtClient, meta);
}

struct metadata* ecLookupMetadata(char* key) {

	struct metadata * meta = zht_lookup_meta(zhtClient,key);
	
	return meta;
}
