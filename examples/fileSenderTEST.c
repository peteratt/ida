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

int main(int argc, char* argv[]) {

	//INPUTS: Filename to Encode, # data blocks (n), # parity blocks (m), size of buffer (in B)
	char * filename = argv[1];
	int k = atoi(argv[2]);
	int m = atoi(argv[3]);
	int bufsize = atoi(argv[4]);


	ida_init("../src/zhtNeighborFile", "../lib/ZHT/zht.cfg");

	struct metadata* meta = ecFileEncode(filename, k, m, bufsize,GIBRALTAR);
	
	struct comLocations loc;
	ecFileSend(filename, k, m, &loc);
	
	printf("Reaches here 1\n");
	ecInsertMetadata(meta);
	printf("Reaches here 2\n");

	free_struct_comLocations(&loc);//Free the structure
	
	ida_finalize();
	
	return 0;
}
