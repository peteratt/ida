/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 *
 * File Decoder Example for Erasure-Coding Libraries
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <ffsnet_bridger.h>
#include <ec.h>

#include "../inc/benchmark.h"

int main(int argc, char* argv[]) {

	//freopen( "stderr.log", "w", stderr );
	
	struct timeval tvBegin, tvReceiving, tvEnd, t1, t2, totalTime;
	
	// Initial timestamp
	gettimeofday(&tvBegin, NULL);
	timeval_print(&tvBegin);

	//INPUTS: Filename to Decode
	char * filename = argv[1];

	ida_init("../src/zhtNeighborFile", "../lib/ZHT/zht.cfg");
	
	struct metadata* meta = ecLookupMetadata(filename);
	
	ecFileReceive(filename, meta->k, meta->m, meta->loc);
	
	// Timestamp after receiving, to retrieve overhead
	gettimeofday(&tvReceiving, NULL);
	timeval_print(&tvReceiving);

	ecFileDecode(filename, meta);
	
	// Take final timestamp
	gettimeofday(&tvEnd, NULL);
	timeval_print(&tvEnd);
	
	// Compute throughput:
	timeval_subtract(&t1, &tvReceiving, &tvBegin);
	timeval_subtract(&t2, &tvEnd, &tvReceiving);
	timeval_subtract(&totalTime, &tvEnd, &tvBegin);
	
	double throughputReceiving = meta->fileSize / (double)(1000000 * t1.tv_sec + t1.tv_usec);
	double throughputDecoding = meta->fileSize / (double)(1000000 * t2.tv_sec + t2.tv_usec);
	double throughputTotal = meta->fileSize / (double)(1000000 * totalTime.tv_sec + totalTime.tv_usec);
	
	// printf's separated by commas for the CSV
	printf("%s,%lu,%f,%f,%f\n", meta->filename, meta->fileSize, throughputReceiving, throughputDecoding, throughputTotal);
	
	free_struct_comLocations(meta->loc);//Free the structure
	free(meta->loc);
	free(meta);
	
	ida_finalize();
	return 0;
}
