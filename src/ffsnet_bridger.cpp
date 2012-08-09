/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 *
 */
#include "ffsnet.h"
#include "ffsnetCPP.h"

int * Transfer_init_c(UDTArray_c * SsocksP, struct metadata * meta, int operation){
	UDTArray * SsocksCPP_P = (UDTArray *) SsocksP;
	
	int * res = Transfer_init(SsocksCPP_P, meta, operation);
	*SsocksP = (UDTArray_c) (*SsocksCPP_P);
	
	return res;
}

int bufferSend_c(UDTArray_c Ssocks, int index, unsigned char * buffer){
	UDTArray SsocksCPP = (UDTArray) Ssocks;
	
	return bufferSend(SsocksCPP, index, buffer);
}

int bufferRecv_c(UDTArray_c Ssocks, int index, unsigned char * buffer){
	UDTArray SsocksCPP = (UDTArray) Ssocks;
	
	return bufferRecv(SsocksCPP, index, buffer);
}

int Transfer_destroy_c(UDTArray_c Ssocks){
	UDTArray SsocksCPP = (UDTArray) Ssocks;
	
	return Transfer_destroy(SsocksCPP);
}


