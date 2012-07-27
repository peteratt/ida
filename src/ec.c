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
	FILE *destinationMeta;

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

	/* Open source file and destination files */
	source = fopen(filename, "rb");
	if(!source){
		printf("ERROR: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	int j;
	
	for (j = 0; j < k + m; j++) {
		sprintf(filenameDest, "./%s.%d", filename, j);
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
	sprintf(filenameDest, "./%s.meta", filename);
    	destinationMeta = fopen(filenameDest, "w");
	if(!destinationMeta){
		printf("ERROR: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	struct metadata* meta = (struct metadata*)malloc(sizeof(struct metadata));

	//Filename
	fprintf(destinationMeta,"%s\n",filename);
	meta->filename = filename;
	//FileSize
	fprintf(destinationMeta,"%ld\n",ftell(source));
	meta->fileSize = ftell(source);
	//Time of Encoding
	time_t rawtime;
	time ( &rawtime );
  	fprintf (destinationMeta,"%s", ctime (&rawtime) );
	//Parameters
	fprintf(destinationMeta,"%i %i\n", k, m);
	meta->k = k;
	meta->m = m;
	//Type of encoding
	meta->encodingLib = libraryId;
	
	//Buffer size
	fprintf(destinationMeta,"%i\n",bufsize);
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

int ecFileDecode(char *filename) {
	//INPUTS: Filename to Decode
	char filenameDest[260];	
	int bufsize,n,m,filesize,ngood,nbad,j,i,unfinished,nBytes,tmp, towrite;

	/* Load information from metaFile */
	FILE *sourceMeta;
	char loadedInfo[260];

	sprintf(filenameDest, "./%s.meta", filename);
	sourceMeta = fopen(filenameDest, "r");
	if(!sourceMeta){
		printf("ERROR: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	//Read Filename
	fgets(loadedInfo, 260, sourceMeta);
	//printf("%s",loadedInfo);
	//Read Filesize
	fscanf(sourceMeta,"%i",&filesize);
	printf("%i\n",filesize);
	fgets(loadedInfo, 260, sourceMeta);
	//Read Time
	fgets(loadedInfo, 260, sourceMeta);
	//printf("%s",loadedInfo);
	//Read m and n
	fscanf(sourceMeta,"%i %i\n",&n,&m);
	printf("%i %i\n",n,m);
	//Read protocol
	//TODO
	//Read bufsize
	fscanf(sourceMeta,"%i\n",&bufsize);
	printf("%i\n",bufsize);



	/* Initialize Ec Functions and Context */
	ecFunctions ec;
    ecContext context;

	//TODO WE NEED THE LIBRARY HERE
	int libraryId = 0;
	ec_init_Library(libraryId, &ec);
	//ec_init_Gibraltar(&ec); //If Gibraltar
	//ec_init_JerasureRS(&ec); //if Jerasure Reed Solomon
	
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
		sprintf(filenameDest, "./%s.%d", filename, j);
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
	fclose(sourceMeta);
	fclose(destination);
	for (j = 0; j < ngood; j++) {
		fclose(source[goodBufIds[j]]);
	}
	
	/* Free allocated memory and destroy Context */
	ec->free(buffers, context);
    	ec->destroy(context);
	
	return EXIT_SUCCESS;
}

int getSendLocations(char * filehash, struct comLocations * loc, int minimum){
	
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
	for (i = 0; i < blocksNumber; i++) {
				
		chunknameLen = sprintf(chunkname, "%s.%d", filehash, i);
		current->distantChunkName = (char *) malloc(chunknameLen+1);
		strcpy(current->distantChunkName,chunkname);
		
		current->localChunkName = (char *) malloc(chunknameLen+1);
		strcpy(current->localChunkName,chunkname);
		
		current = current->next;
	}
	
	loc->locationsNumber = blocksNumber; // We actually tell the program how many blocks we can transfer
	
	return 0;

}


int getRecvLocations(char * filehash, struct comLocations * loc, int minimum){
	
	//Read into the metadata
	
	/*
	int n = loc->locationsNumber;
	int currentLocationsNumber = 0;
	int i;
	
	int FFSNETPORTSTART = 9001;
	char chunkname[128];
	
	struct comTransfer * prev = NULL;
	struct comTransfer * current;
	//1. We acquire the locations [Static Here], Dynamic is TODO
	//Currently the list implementation makes the most important the LAST one of the list.
	
	for (i = 0; i < n; i++) {
		
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
	}
	
	loc->transfers = prev;
	
	//2. We check that locations are enough to reconstruct the file. If not, exit. If yes, adjust the actual number of locations that we return. (should be between minimum and the original locationsNumber)
	if(currentLocationsNumber < minimum){
		return 1; //That should be defined as a constant: NOTENOUGHLOCATIONS
	}
	loc->locationsNumber = currentLocationsNumber;
	*/
	
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
		ffs_sendfile_c("udt", curTransfer->hostName, port_str, curTransfer->distantChunkName, curTransfer->localChunkName);
		
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
	
	return 0;
}

void * threadRecvFunc(void * args){
		struct comTransfer * curTransfer = (struct comTransfer *)args;
		
		char port_str[10];
		sprintf(port_str, "%d", curTransfer->port);
		
		printf("Receiving: host: %s; port:%s; distantName: %s; localName: %s \n",curTransfer->hostName, port_str, curTransfer->distantChunkName, curTransfer->localChunkName);
		ffs_recvfile_c("udt", curTransfer->hostName, port_str, curTransfer->localChunkName, curTransfer->distantChunkName);
		
		return NULL;
};

int ecFileReceive(char *filename, int k, int m, struct comLocations * loc) {
	int n = k + m;
	int i;
	
	
	pthread_t threads[n];
	
	loc->locationsNumber = n; 
	
	getRecvLocations(filename,loc,k); // the minimum we need is k (no worries if we get less locations)
	
	struct comTransfer * curTransfer = loc->transfers;
	
	for (i = 0; i < loc->locationsNumber; i++) {
		// Send the chunks through UDT to the server
		
		// TODO: Needs error treatment
		pthread_create(&threads[i], NULL, &threadRecvFunc, (void *)curTransfer);
		curTransfer = curTransfer->next;
	}
	
	for (i = 0; i < loc->locationsNumber; i++) {
		pthread_join(threads[i], NULL);
	}
	
	return 0;
}

int ecInsertMetadata(struct metadata* meta) {
	
	const char* parsedPackage = zht_parse_meta(meta);
	
	int iret = c_zht_insert2_std(zhtClient, meta->filename, parsedPackage);
	//int iret2 = c_zht_insert2_std(zhtClient, "hello", "zht");
	fprintf(stderr, "c_zht_insert, return code: %d\n", iret);
	//fprintf(stderr, "c_zht_insert, return code: %d\n", iret2);
	
	// TESTING
	/*char* result = (char*) calloc(LOOKUP_SIZE, sizeof(char));
	
	if (result != NULL) {
		size_t n;
		int lret = c_zht_lookup2_std(zhtClient, meta->filename, result, &n);
		fprintf(stderr, "c_zht_lookup, return code: %d\n", lret);
		fprintf(stderr, "c_zht_lookup, return value, %s\n",	result);
		//int lret2 = c_zht_lookup2_std(zhtClient, "hello", result, &n);
		//fprintf(stderr, "c_zht_lookup, return code: %d\n", lret2);
		//fprintf(stderr, "c_zht_lookup, return value, %s and size: %lu\n", result, n);
	}
	
	struct metadata* lookedup = zht_unparse_meta(result);
	fprintf(stderr, "Filename retrieved: %s\n",	lookedup->filename);
	free(result);*/
	// END TESTING
	
	return 0;
}

struct metadata* ecLookupMetadata(char* key) {

	char* result = (char*) calloc(LOOKUP_SIZE, sizeof(char));
	
	if (result != NULL) {
		size_t n;
		int lret = c_zht_lookup2_std(zhtClient, key, result, &n);
		fprintf(stderr, "c_zht_lookup, return code: %d\n", lret);
		fprintf(stderr, "c_zht_lookup, return value, %s\n",	result);
		//int lret2 = c_zht_lookup2_std(zhtClient, "hello", result, &n);
		//fprintf(stderr, "c_zht_lookup, return code: %d\n", lret2);
		//fprintf(stderr, "c_zht_lookup, return value, %s and size: %lu\n", result, n);
	}
	
	struct metadata* lookedup = zht_unparse_meta(result);
	fprintf(stderr, "Filename retrieved: %s\n",	lookedup->filename);
	free(result);

	return lookedup;
}
