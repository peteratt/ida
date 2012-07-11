#include <ec.h>
#include "../inc/ffsnet.h"

int ecFileEncode(char *filename, int k, int m, int bufsize) {
	
	//INPUTS: Filename to Encode, # data blocks (n), # parity blocks (m), size of buffer (in B)

	char filenameDest[256];

	FILE *source;
	FILE *destination[k+m];
	FILE *destinationMeta;

	/* Initialize Ec Functions and Context */
	ecFunctions ec;
    ecContext context;

	ec_init_Gibraltar(&ec); //If Gibraltar
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

	//Filename
	fprintf(destinationMeta,"%s\n",filename);
	//FileSize
	fprintf(destinationMeta,"%ld\n",ftell(source));
	//Time of Encoding
	time_t rawtime;
	time ( &rawtime );
  	fprintf (destinationMeta,"%s", ctime (&rawtime) );
	//Parameters
	fprintf(destinationMeta,"%i %i\n", k, m);
	//Type of encoding
	//TODO
	//Buffer size
	fprintf(destinationMeta,"%i\n",bufsize);

	/* Close files */
	fclose(source);
	for (j = 0; j < k + m; j++) {
		fclose(destination[j]);
	}

	/* Free allocated memory and destroy Context */
	ec->free(buffers, context);
    	ec->destroy(context);
	
	return EXIT_SUCCESS;
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

	ec_init_Gibraltar(&ec); //If Gibraltar
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

int ecFileSend(char *filename, int k, int m) {
	int n = k + m;
	char chunkname[128];
	int i;
	
	for (i = 1; i <= n; i++) {
		// Register the chunks
		sprintf(chunkname, "%s.%d", filename, i);

		// Send them right after through UDT to the server
		// TODO: Needs error treatment
		//ffs_sendfile("udt", "127.0.0.1", "9001", chunkname, chunkname);
	}
	
	return 0;
}

int ecFileReceive(char *filename, int k, int m) {
	char chunkname[128];
	int i;
	
	for (i = 1; i <= k; i++) {
		// Register the chunks
		sprintf(chunkname, "%s.%d", filename, i);

		// Receive them right after through UDT from the server
		// TODO: Needs error treatment
		//ffs_recvfile("udt", "127.0.0.1", "9001", chunkname, chunkname);
	}
	
	return 0;
}
