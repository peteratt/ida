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

	struct metadata* meta = ecFileEncode(filename, k, m, bufsize,GIBRALTAR);
	ecInsertMetadata("../src/zhtNeighborFile", "../lib/ZHT/zht.cfg", meta);
	free(meta);
	
	return ecFileSend(filename, k, m);
}
