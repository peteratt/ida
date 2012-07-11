/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * Defines the Jerasure compatibility interface. Provides different types of Jerasure
 */

#include "ec_interface.h"
#ifndef JERASURECOMPATIBILITY_H_
#define JERASURECOMPATIBILITY_H_

#if __cplusplus
extern "C" {
#endif /* __cplusplus */

int jerasureRS_init_interface( int n, int m, ecContext *c );
int jerasure_destroy_interface ( ecContext c );
int jerasure_alloc_interface( void **buffers, int buf_size, int *ld, ecContext c );
int jerasure_free_interface( void *buffers, ecContext c );
int jerasure_generate_interface( void *buffers, int buf_size, ecContext c );
int jerasure_recover_interface( void *buffers, int buf_size, int *buf_ids, int recover_last,ecContext c );


int ec_init_JerasureRS(ecFunctions * ec);
//int ec_init_JerasureCRS(ecFunctions * ec);

#ifdef __cplusplus
}
#endif

#endif /*JERASURECOMPATIBILITY_H_*/
