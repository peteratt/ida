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
	
<<<<<<< HEAD
	struct metadata* meta = ecFileEncode(filename, k, m, bufsize);
	ecInsertMetadata("../src/zhtNeighborFile", "../lib/ZHT/zht.cfg", meta);
	free(meta);
=======
	ecFileEncode(filename, k, m, bufsize, GIBRALTAR);
	//ecInsertMetadata("neighbors", "config");
>>>>>>> 88c6ec485d2b9123585ca1d2f8222bda8637e05a
	return ecFileSend(filename, k, m);
}
