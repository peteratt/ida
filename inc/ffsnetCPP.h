/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * Author:  Pedro Alvarez-Tabio
 * Email:   palvare3@iit.edu
 *
 */
#include "ec.h"
#include <udt.h>
 
struct UDTArray_s{
	int * indexArray;
	UDTSOCKET * socks;
	struct metadata * meta;
	int operation;
	int socknumber;
};

typedef struct UDTArray_s * UDTArray;

int * Transfer_init(UDTArray * SsocksP, struct metadata * meta, int operation);
int bufferSend(UDTArray Ssocks, int index, unsigned char * buffer, int bufsize);
int bufferRecv(UDTArray Ssocks, int index, unsigned char * buffer, int bufsize);
int Transfer_destroy(UDTArray Ssocks);
