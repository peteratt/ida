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

#include "../inc/ffsnet.h"

int main(int argc, char* argv[]) {
	//INPUTS: Filename to Decode
	char * filename = argv[1];
	int k = 4;
	int m = 2;
	
	ecFileReceive(filename, k, m);
	return ecFileDecode(filename);
}
