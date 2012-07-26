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
	
	ecFileReceive(filename, meta->k, meta->m, meta->locations);
	free(meta);
	return ecFileDecode(filename);
}
