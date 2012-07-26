/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 *
 * File Decoder Example for Erasure-Coding Libraries
 */

#include <ec.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <ffsnet_bridger.h>

int main(int argc, char* argv[]) {
	//INPUTS: Filename to Decode
	char * filename = argv[1];
	struct metadata* meta = ecLookupMetadata(filename);
	
	ida_init("../src/zhtNeighborFile", "../lib/ZHT/zht.cfg");
	
	//META here
	struct comLocations loc;
	struct metadata* meta;
	
	ecFileReceive(filename, meta->k, meta->m, &loc);
	free(meta);

	ecFileDecode(filename);
	
	free_struct_comLocations(&loc);//Free the structure
	
	ida_finalize();
	return 0;
}
