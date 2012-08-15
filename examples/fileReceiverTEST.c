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

#include <ec.h>
#include <ida.h>

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
	
	ida_download(filename);
	
	// Take final timestamp
	gettimeofday(&tvEnd, NULL);
	timeval_print(&tvEnd);
	
	// Compute throughput:
	timeval_subtract(&totalTime, &tvEnd, &tvBegin);
	
	// Filesize
	FILE *file;
	file = fopen(filename, "rb");
	if(!file){
		dbgprintf("Metadata Fill ERROR: %s\n", strerror(errno));
		exit(EXIT_FAILURE);//maybe a return 1 could be appropriate?
	}
	fseek(file, 0L, SEEK_END);
	int fileSize = ftell(file);
	fclose(file);
	
	double throughputTotal = fileSize / (1000000 * totalTime.tv_sec + totalTime.tv_usec);
	
	// printf's separated by commas for the CSV
	printf("%s,%lu,%f\n", filename, fileSize, throughputTotal);
	
	ida_finalize();
	return 0;
}
