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

#include <ec.h>
#include <ida.h>

#include "../inc/benchmark.h"

int main(int argc, char* argv[]) {

	//freopen( "stderr.log", "w", stderr );
	struct timeval tvBegin, tvEncoding, tvEnd, t1, t2, totalTime;
	
	FILE *results;

	// Initial timestamp
	gettimeofday(&tvBegin, NULL);
	timeval_print(&tvBegin);

	//INPUTS: Filename to Encode, # data blocks (n), # parity blocks (m), size of buffer (in B)
	char * filename = argv[1];
	int k = atoi(argv[2]);
	int m = atoi(argv[3]);
	int bufsize = atoi(argv[4]);

	ida_init("../src/zhtNeighborFile", "../lib/ZHT/zht.cfg");

	ida_send(filename,k,m,bufsize);

	// Take final timestamp
	gettimeofday(&tvEnd, NULL);
	timeval_print(&tvEnd);
	
	// Compute throughput:
	timeval_subtract(&totalTime, &tvEnd, &tvBegin);

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
