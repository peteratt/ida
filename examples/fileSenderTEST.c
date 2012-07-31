/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * Author:  Pedro Alvarez-Tabio
 * Email:   palvare3@iit.edu
 *
 * File Encoder Example for Erasure-Coding Libraries
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <ffsnet_bridger.h>
#include <ec.h>

#include "benchmark.c"

int main(int argc, char* argv[]) {

	struct timeval tvBegin, tvEncoding, tvEnd, t1, t2, totalTime;

	// Initial timestamp
	gettimeofday(&tvBegin, NULL);
	timeval_print(&tvBegin);

	//INPUTS: Filename to Encode, # data blocks (n), # parity blocks (m), size of buffer (in B)
	char * filename = argv[1];
	int k = atoi(argv[2]);
	int m = atoi(argv[3]);
	int bufsize = atoi(argv[4]);

	ida_init("../src/zhtNeighborFile", "../lib/ZHT/zht.cfg");

	struct metadata* meta = ecFileEncode(filename, k, m, bufsize,JERASURERS);
	
	// Timestamp after encoding, to retrieve overhead
	gettimeofday(&tvEncoding, NULL);
	timeval_print(&tvEncoding);
	
	meta->loc = (struct comLocations *) malloc(sizeof(struct comLocations));
	ecFileSend(filename, k, m, meta->loc);
	
	ecInsertMetadata(meta);

	// Take final timestamp
	gettimeofday(&tvEnd, NULL);
	timeval_print(&tvEnd);
	
	// Compute throughput:
	timeval_subtract(&t1, &tvEncoding, &tvBegin);
	timeval_subtract(&t2, &tvEnd, &tvEncoding);
	timeval_subtract(&totalTime, &tvEnd, &tvBegin);
	
	double throughputEncoding = meta->fileSize / (1000000 * t1.tv_sec + t1.tv_usec);
	double throughputSending = meta->fileSize / (1000000 * t2.tv_sec + t2.tv_usec);
	double throughputTotal = meta->fileSize / (1000000 * totalTime.tv_sec + totalTime.tv_usec);
	
	// printf's separated by commas for the CSV
	
	free_struct_comLocations(meta->loc);//Free the structure
	free(meta->loc);
	free(meta);
	
	ida_finalize();
	
	return 0;
}
