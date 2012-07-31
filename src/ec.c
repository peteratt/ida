/*
 * Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * Author:  Pedro Alvarez-Tabio
 * Email:   palvare3@iit.edu
 *
 */


#include   <stdbool.h>

#include "../inc/zht_bridger.h"
#include "../inc/ec.h"
#include "../inc/ffsnet_bridger.h"

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


int ec_init_Library(int libraryId, ecFunctions *ec){

	//TODO this function should take GPU_CAPABLE into consideration

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

struct metadata* ecFileEncode(char *filename, int k, int m, int bufsize, int libraryId){
	
	//INPUTS: Filename to Encode, # data blocks (n), # parity blocks (m), size of buffer (in B), Library ID (see globals.h for available libs)

	char filenameDest[256];

	FILE *source;
	FILE *destination[k+m];

	/* Initialize Ec Functions and Context */
	ecFunctions ec;
    ecContext context;

	ec_init_Library(libraryId, &ec);
	//ec_init_Gibraltar(&ec); //If Gibraltar
	//ec_init_JerasureRS(&ec); //if Jerasure Reed Solomon
	
	int rc = ec->init(k, m, &context);
	if (rc) {
		printf("Error:  %i\n", rc);
		exit(EXIT_FAILURE);
	}

	/* Allocate/define the appropriate number of buffers */
	void *buffers;
	ec->alloc(&buffers, bufsize, &bufsize, context);

	/* Create destination Directory (cache) if doesn't exist */
	char dirName[256];
	sprintf(dirName, "%s%s",CACHE_DIR_PATH,CACHE_DIR_NAME);
	mkdir(dirName, 0777);

	/* Open source file and destination files */
	source = fopen(filename, "rb");
	if(!source){
		printf("ERROR: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	int j;
	
	for (j = 0; j < k + m; j++) {
		sprintf(filenameDest, "%s%s/%s.%d",CACHE_DIR_PATH,CACHE_DIR_NAME, filename, j);
	    destination[j] = fopen(filenameDest, "wb");
		if(!destination[j]){
			printf("ERROR: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	/* Encode N*BUFSIZE bytes at a time then repeat until EOF */
	int bytesRead;
	while(!feof(source)){
		//1-read		
		bytesRead = fread(buffers,sizeof(unsigned char), bufsize*k, source);

		if(bytesRead < bufsize * k){//Padding(with 0s) if necessary
			memset((unsigned char *) buffers + bytesRead, 0, bufsize*k - bytesRead);
		}
		
		//2-encode
    		ec->generate(buffers, bufsize, context);

		//3-Write
		for (j = 0; j < k + m; j++) {
    			fwrite((unsigned char *)buffers + j*bufsize, sizeof(unsigned char), bufsize, destination[j]);
    		}
	}

	/* Writing the metaData */
	struct metadata* meta = (struct metadata*)malloc(sizeof(struct metadata));

	//Filename
	meta->filename = filename;
	//FileSize
	meta->fileSize = ftell(source);
	//Parameters
	meta->k = k;
	meta->m = m;
	//Type of encoding
	meta->encodingLib = libraryId;
	//Buffer size
	meta->bufsize = bufsize;

	/* Close files */
	fclose(source);
	for (j = 0; j < k + m; j++) {
		fclose(destination[j]);
	}

	/* Free allocated memory and destroy Context */
	ec->free(buffers, context);
	ec->destroy(context);
	
	return meta;
}

int ecFileDecode(char *filename, struct metadata * meta) {
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
		printf("Error:  %i\n", rc);
		exit(EXIT_FAILURE);
	}

	/* Allocate/define the appropriate number of buffers */
	void *buffers;
	ec->alloc(&buffers, bufsize, &bufsize, context);


	/* Identify missing files and open files */
	//TODO Check files checksum + length to garantee correctness
	FILE *source[n+m];
	FILE *destination;

	int goodBufIds[m+n];
	int badBufIds[m+n];
	
	ngood=0;
	nbad=0;

	destination = fopen(filename, "wb");
	if(!destination){
		printf("ERROR: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	for (j = 0; j < n + m; j++) {
		sprintf(filenameDest, "%s%s/%s.%d",CACHE_DIR_PATH,CACHE_DIR_NAME, filename, j);
	    	source[j] = fopen(filenameDest, "rb");
		if(!source[j]){
			if(errno != ENOENT && errno != EACCES){
			//Its normal not to find some files TODO Note: In a real application, the system should be aware of the failures, and should try in priority the known good parts.
				printf("ERROR: %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
			if(j < n) badBufIds[nbad++] = j;
		}
		else{
			goodBufIds[ngood] = j;

			ngood++;
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
	
	while (unfinished) {
		for (j = 0; j < ngood; j++) {
			fread(buffers + j*bufsize,sizeof(unsigned char), bufsize, source[goodBufIds[j]]);
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
	//fclose(sourceMeta);
	fclose(destination);
	for (j = 0; j < ngood; j++) {
		fclose(source[goodBufIds[j]]);
	}
	
	for(j=0; j < ngood; j++){
		sprintf(filenameDest, "%s%s/%s.%d",CACHE_DIR_PATH,CACHE_DIR_NAME, filename, j);
		remove(filenameDest);
	}
	
	/* Free allocated memory and destroy Context */
	ec->free(buffers, context);
    	ec->destroy(context);
	
	return EXIT_SUCCESS;
}

int randomStr(char * destination, int destLen){
	char * password_chars = "1234567890abcdefghijklmnopqrstuvwxyz";
	unsigned int iseed = (unsigned int)time(NULL);
 	srand (iseed);
	int i;
	int random;

	for (i = 0; i < destLen; i++) {
		random = rand()%(strlen(password_chars)-1);
		destination[i] = password_chars[random];
	}
	destination[destLen-1] = '\0';
	
	return 0;
}


int getSendLocations(char * filename, struct comLocations * loc, int minimum){
	
	int blocksNumber = loc->locationsNumber; //blocksNumber is the number of actual data blocks available
	int i;
	
	char chunkname[256];
	unsigned int chunknameLen;
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
		
	
	for (i = blocksNumber-1; i >= 0; i--) {	
		
		chunknameLen = sprintf(chunkname, "%s.%d", filehash, i);
		current->distantChunkName = (char *) malloc(chunknameLen+1);
		strcpy(current->distantChunkName,chunkname);
		
		chunknameLen = sprintf(chunkname, "%s%s/%s.%d",CACHE_DIR_PATH,CACHE_DIR_NAME, filename, i);
		current->localChunkName = (char *) malloc(chunknameLen+1);
		strcpy(current->localChunkName,chunkname);
		
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
		free(current->localChunkName);
		
		tofree = current;
		current = current->next;
		free(tofree);
	}
}


void * threadSendFunc(void * args){
		struct comTransfer * curTransfer = (struct comTransfer *)args;
		
		char port_str[10];
		sprintf(port_str, "%d", curTransfer->port);
		
		printf("Sending: host: %s; port:%s; distantName: %s; localName: %s \n",curTransfer->hostName, port_str, curTransfer->distantChunkName, curTransfer->localChunkName);
		ffs_sendfile_c("udt", curTransfer->hostName, port_str,curTransfer->localChunkName, curTransfer->distantChunkName);
		
		return NULL;
};

int ecFileSend(char *filename, int k, int m, struct comLocations * loc) {
	int n = k + m;
	int i;
	
	pthread_t threads[n];
	
	loc->locationsNumber = n;
	
	getSendLocations(filename,loc,n);
	
	struct comTransfer * curTransfer = loc->transfers;
	
	for (i = 0; i < loc->locationsNumber; i++) {
		// Send the chunks through UDT to the server
		
		// TODO: Needs error treatment
		pthread_create(&threads[i], NULL, &threadSendFunc, (void *)curTransfer);
		curTransfer = curTransfer->next;
	}
	
	for (i = 0; i < loc->locationsNumber; i++) {
		pthread_join(threads[i], NULL);
	}
	
	char filenameDest[256];
	for(i=0; i < loc->locationsNumber; i++){
		sprintf(filenameDest, "%s%s/%s.%d",CACHE_DIR_PATH,CACHE_DIR_NAME, filename, i);
		remove(filenameDest);
	}
	
	return 0;
}

void * threadRecvFunc(void * args){
		struct comTransfer * curTransfer = (struct comTransfer *)args;
		int * retval = (int *) malloc(sizeof(int));
		
		char port_str[10];
		sprintf(port_str, "%d", curTransfer->port);
		
		printf("Receiving: host: %s; port:%s; distantName: %s; localName: %s \n",curTransfer->hostName, port_str, curTransfer->distantChunkName, curTransfer->localChunkName);
		*retval = ffs_recvfile_c("udt", curTransfer->hostName, port_str, curTransfer->distantChunkName, curTransfer->localChunkName);
		
		pthread_exit((void *)retval);
		
		return 0;
};

int ecFileReceive(char *filename, int k, int m, struct comLocations * loc) {
	int n = k + m;
	int i;
	
	
	pthread_t threads[k];
	
	loc->locationsNumber = n; 
	
	getRecvLocations(filename,loc,k); // the minimum we need is k (no worries if we get less locations)
	
	/* Create destination Directory (cache) if doesn't exist */
	char dirName[256];
	sprintf(dirName, "%s%s",CACHE_DIR_PATH,CACHE_DIR_NAME);
	mkdir(dirName, 0777);
	
	
	struct comTransfer * curTransfer = loc->transfers;
	
	for (i = 0; i < k; i++) {
		// Receive the chunks from the servers
		
		// TODO: Needs error treatment
		pthread_create(&threads[i], NULL, &threadRecvFunc, (void *)curTransfer);
		curTransfer = curTransfer->next;
	}
	
	

	int failedTransfers = 0;
	
	for (i = 0; i < k; i++) {
		void * thread_retval;
		pthread_join(threads[i], &thread_retval);
		
		failedTransfers -= *((int *)thread_retval); //retval for fail is -1
		free(thread_retval);
	}

	
	if(failedTransfers > 0){
		while(failedTransfers > 0 && curTransfer != NULL){
			char port_str[10];
			sprintf(port_str, "%d", curTransfer->port);
			
			int retFFS;
			printf("Receiving(parity): host: %s; port:%s; distantName: %s; localName: %s \n",curTransfer->hostName, port_str, curTransfer->distantChunkName, curTransfer->localChunkName);
			retFFS = ffs_recvfile_c("udt", curTransfer->hostName, port_str, curTransfer->distantChunkName, curTransfer->localChunkName);
			
			if(retFFS == 0){
				failedTransfers--;
			}

			
			curTransfer = curTransfer->next;
		}

	}
	
	if(failedTransfers > 0) return 1; //NOT ENOUGH CHUNKS
	
	return 0;
}

int ecInsertMetadata(struct metadata* meta) {
	
	return zht_insert_meta(zhtClient, meta);
}

struct metadata* ecLookupMetadata(char* key) {

	struct metadata * meta = zht_lookup_meta(zhtClient,key);
	
	return meta;
}
