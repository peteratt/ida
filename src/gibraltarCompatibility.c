/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * Defines the Gibraltar compatibility interface
 */

#include "../inc/gibraltarCompatibility.h"
#include "../lib/libgibraltar-1.0/inc/gibraltar.h"
#include <stdlib.h>

int gib_init_interface( int n, int m, ecContext *c ){
	*c = (ecContext) malloc(sizeof(struct ecContext_s));
	if (c == NULL) return 1;//TODO error return for handling
	
	(*c)->n = n;
	(*c)->m = m;
	(*c)->lib_id = 0;			
	//(*c)->lib_context = (gib_context) (*c)->lib_context;	

	gib_context tmp_context;
	int res =  gib_init(n,m,&tmp_context);

	(*c)->lib_context = tmp_context;
	return res;
}

int gib_destroy_interface( ecContext c ){
	gib_destroy((gib_context)(c->lib_context));
	free(c);
	return 0; //TODO error return for handling
}

int gib_alloc_interface( void **buffers, int buf_size, int *ld, ecContext c ){
	return gib_alloc ( buffers, buf_size, ld, (gib_context)(c->lib_context) );
}

int gib_free_interface( void *buffers, ecContext c ){
	return gib_free(buffers, (gib_context)(c->lib_context));
}

int gib_generate_interface( void *buffers, int buf_size, ecContext c ){
	return gib_generate( buffers, buf_size, (gib_context)(c->lib_context));
}

int gib_recover_interface( void *buffers, int buf_size, int *buf_ids, int recover_last,ecContext c ){
	return gib_recover(buffers, buf_size, buf_ids, recover_last, (gib_context)(c->lib_context));
}

int ec_init_Gibraltar(ecFunctions *ec){
	*ec = (ecFunctions) malloc(sizeof(struct ecFunctions_s));
	if (ec == NULL) return 1;//TODO error return for handling

	(*ec)->init = &gib_init_interface;
	(*ec)->destroy = &gib_destroy_interface;
	(*ec)->alloc = &gib_alloc_interface;
	(*ec)->free = &gib_free_interface;
	(*ec)->generate = &gib_generate_interface;
	(*ec)->recover = &gib_recover_interface;
	return 0;
}
