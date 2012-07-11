/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * Defines the Jerasure compatibility interface. Provides different types of Jerasure
 */

#include "../inc/jerasureCompatibility.h"
#include "../lib/Jerasure-1.2/jerasure.h"
#include "../lib/Jerasure-1.2/reed_sol.h"
#include <stdlib.h>



int jerasureRS_init_interface( int n, int m, ecContext *c ){
	*c = (ecContext) malloc(sizeof(struct ecContext_s));
	if (c == NULL) return 1;//TODO error return for handling
	
	(*c)->n = n;
	(*c)->m = m;
	(*c)->lib_id = 12;
	
	//Jerasure doesnt need any context
	return 0;
}
int jerasure_destroy_interface ( ecContext c ){
	free(c);
	return 0;
}

int jerasure_alloc_interface( void **buffers, int buf_size, int *ld, ecContext c ){
	*ld = buf_size;
	*buffers = malloc((c->n+c->m)*buf_size);
	
	if (*buffers == NULL) return 1;
	
	return 0;
}
int jerasure_free_interface( void *buffers, ecContext c ){
	free(buffers);
	return 0;
}
int jerasure_generate_interface( void *buffers, int buf_size, ecContext c ){
	char *data[256];
	char *coding[256];
	int i;
	for (i = 0; i < (c->n); i++) {
		data[i] = ((char *)buffers) + i*buf_size;
	}
	for (i = 0; i < (c->m); i++) {
		coding[i] = ((char *)buffers) + (i+(c->n))*buf_size;
	}
	
	//a switch here could let choose the correct matrix depending on c->lib_id
	jerasure_matrix_encode(c->n, c->m, 8, reed_sol_vandermonde_coding_matrix(c->n, c->m, 8), data, coding, buf_size);
	return 0;
}

int jerasure_recover_interface( void *buffers, int buf_size, int *buf_ids, int recover_last,ecContext c ){

	/* NOTE(from Gibraltar) Gibraltar does not want to necessarily recover ALL of the lost buffers, as
   * sometimes this is not necessary or convenient.  However, Jerasure does 
   * want to recover all buffers, and will reference any intact buffers that it
   * pleases.  The bulk of the logic below is to address this difference.
   */

	char *data[256];
	char *coding[256];
	int erasures[256];
	int missing[256];
	int i;

	for (i = 0; i < c->n+c->m; i++)
		missing[i] = 1;

	for (i = 0; i < c->n+recover_last; i++) {
		if (buf_ids[i] < c->n) {
			data[buf_ids[i]] = (char *)buffers + i*buf_size;
			missing[buf_ids[i]] = 0;
		} else {
			coding[buf_ids[i] - c->n] = (char *)buffers + i*buf_size;
			missing[buf_ids[i]] = 0;
		}
	}
	
	int counter = 0;
	for (i = c->n; i < c->n+recover_last; i++) {
		erasures[counter] = buf_ids[i];
		counter++;
	}

	int offset = c->n+recover_last;
	for (i = 0; i < c->n+c->m; i++)
		if (missing[i]) {
			erasures[counter] = i;
			counter++;
			if (i < c->n)
				data[i] = (char *)buffers + offset*buf_size;
			else
				coding[i - c->n] = (char *)buffers + offset*buf_size;
			offset++;
		}
	erasures[counter] = -1;
	//a switch here could let choose the correct matrix depending on c->lib_id
	jerasure_matrix_decode(c->n, c->m, 8, reed_sol_vandermonde_coding_matrix(c->n, c->m, 8), 0, erasures, data, coding, buf_size);
	
	return 0;
}

int ec_init_JerasureRS(ecFunctions * ec){
	*ec = (ecFunctions) malloc(sizeof(struct ecFunctions_s));
	if (ec == NULL) return 1;//TODO error return for handling

	(*ec)->init = &jerasureRS_init_interface;
	(*ec)->destroy = &jerasure_destroy_interface;
	(*ec)->alloc = &jerasure_alloc_interface;
	(*ec)->free = &jerasure_free_interface;
	(*ec)->generate = &jerasure_generate_interface;
	(*ec)->recover = &jerasure_recover_interface;
	return 0;
}
