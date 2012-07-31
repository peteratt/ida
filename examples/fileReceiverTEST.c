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

	freopen( "stderr.log", "w", stderr );

	//INPUTS: Filename to Decode
	char * filename = argv[1];

	ida_init("../src/zhtNeighborFile", "../lib/ZHT/zht.cfg");
	
	struct metadata* meta = ecLookupMetadata(filename);
	
	ecFileReceive(filename, meta->k, meta->m, meta->loc);

	ecFileDecode(filename, meta);
	
	free_struct_comLocations(meta->loc);//Free the structure
	free(meta->loc);
	free(meta);
	
	ida_finalize();
	return 0;
}
