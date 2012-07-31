/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * Author:  Pedro Alvarez-Tabio
 * Email:   palvare3@iit.edu
 *
 */
#ifndef IDA_GLOBALS_H_
#define IDA_GLOBALS_H_ 

 
#include "ec.h"
#include <malloc.h>
 
int ida_send(char * filename, int k, int m, int bufsize){
	struct metadata* meta = ecFileEncode(filename, k, m, bufsize,JERASURERS);
	
	meta->loc = (struct comLocations *) malloc(sizeof(struct comLocations));
	ecFileSend(filename, k, m, meta->loc);
	
	ecInsertMetadata(meta);

	free_struct_comLocations(meta->loc);//Free the structure
	free(meta->loc);
	free(meta);

	return 0;
}
int ida_download(char * filename){

	struct metadata* meta = ecLookupMetadata(filename);
	
	ecFileReceive(filename, meta->k, meta->m, meta->loc);

	ecFileDecode(filename, meta);
	
	free_struct_comLocations(meta->loc);//Free the structure
	free(meta->loc);
	free(meta);

	return 0;
}

#endif // IDA_GLOBALS_H_ 
