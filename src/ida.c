/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * Author:  Pedro Alvarez-Tabio
 * Email:   palvare3@iit.edu
 *
 */
#include "ec.h"
#include <malloc.h>
 
int ida_send(char * filename, int k, int m, int bufsize){

	struct metadata  * meta;
	ecfillmeta(filename, filename, k, m , GIBRALTAR, bufsize, &meta); //currently virtual name = physical name
	
	ecFileEncode(meta);
	
	ecInsertMetadata(meta);

	free_struct_comLocations(meta->loc);//Free the structure
	free(meta->loc);
	free(meta);

	return 0;
}
int ida_download(char * filename){

	struct metadata* meta = ecLookupMetadata(filename);//filename = virtual path here (key)
	exit(1);
	ecFileDecode(filename, meta);//filename = physical path here
	
	free_struct_comLocations(meta->loc);//Free the structure
	free(meta->loc);
	free(meta);

	return 0;
}

