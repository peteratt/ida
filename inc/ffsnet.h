/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 *
 */

#ifndef _FFSNET_H_
#define _FFSNET_H_

#define SERVER_RECVFILE 0
#define CLIENT_SENDBUF 0

#define SERVER_SENDFILE 1
#define CLIENT_RECVBUF 1

 
#ifdef __cplusplus
extern "C" {
#endif


typedef void * UDTArray_c;

int * Transfer_init_c(UDTArray_c * Ssocks, struct metadata * meta, int operation);
int bufferSend_c(UDTArray_c Ssocks, int index, unsigned char * buffer, int bufsize);
int bufferRecv_c(UDTArray_c Ssocks, int index, unsigned char * buffer, int bufsize);
int Transfer_destroy_c(UDTArray_c Ssocks); 

#ifdef __cplusplus
}
#endif

#endif
