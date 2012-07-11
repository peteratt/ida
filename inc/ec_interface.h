/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * Defines the interface wrapper for libraries
 */

#ifndef ECINTERFACE_H_
#define ECINTERFACE_H_

struct ecContext_s {
	int n, m;
	void *lib_context;
	char *optParameters;
	int lib_id;
};

typedef struct ecContext_s* ecContext;

struct ecFunctions_s {
	int (*init) ( int n, int m, ecContext *c );
	int (*destroy) ( ecContext c );
	int (*alloc) ( void **buffers, int buf_size, int *ld, ecContext c );
	int (*free) ( void *buffers, ecContext c );
	int (*generate) ( void *buffers, int buf_size, ecContext c );
	int (*recover) ( void *buffers, int buf_size, int *buf_ids, int recover_last,ecContext c );
};


typedef struct ecFunctions_s* ecFunctions;

#endif /*ECINTERFACE_H_*/
