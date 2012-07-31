/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * Author:  Pedro Alvarez-Tabio
 * Email:   palvare3@iit.edu
 *
 * File Encoder Example for Erasure-Coding Libraries
 */

#include <ida.h>
#include <ec.h>


int main(int argc, char* argv[]) {

	//freopen( "stderr.log", "w", stderr );

	//INPUTS: Filename to Encode, # data blocks (n), # parity blocks (m), size of buffer (in B)
	char * filename = argv[1];
	int k = atoi(argv[2]);
	int m = atoi(argv[3]);
	int bufsize = atoi(argv[4]);


	ida_init("../src/zhtNeighborFile", "../lib/ZHT/zht.cfg");

	ida_send(filename,k,m,bufsize);
	
	ida_finalize();
	return 0;
}
