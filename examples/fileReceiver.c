/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 *
 * File Decoder Example for Erasure-Coding Libraries
 */

#include <ida.h>
#include <ec.h>

int main(int argc, char* argv[]) {

	//INPUTS: Filename to Decode
	char * filename = argv[1];

	ida_init("../src/zhtNeighborFile", "../lib/ZHT/zht.cfg");
	
	ida_download(filename);
	
	ida_finalize();
	return 0;
}
