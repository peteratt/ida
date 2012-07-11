/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * Defines the Gibraltar compatibility interface
 */

#include "ec_interface.h"
#ifndef GIBRALTARCOMPATIBILITY_H_
#define GIBRALTARCOMPATIBILITY_H_
 
#if __cplusplus
extern "C" {
#endif /* __cplusplus */

int gib_init_interface( int n, int m, ecContext *c );
int gib_destroy_interface( ecContext c );
int gib_alloc_interface( void **buffers, int buf_size, int *ld, ecContext c );
int gib_free_interface( void *buffers, ecContext c );
int gib_generate_interface( void *buffers, int buf_size, ecContext c );
int gib_recover_interface( void *buffers, int buf_size, int *buf_ids, int recover_last,ecContext c );

int ec_init_Gibraltar(ecFunctions * ec);

#ifdef __cplusplus
}
#endif

#endif /*GIBRALTARCOMPATIBILITY_H_*/
