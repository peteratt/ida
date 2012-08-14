/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 *
 */

#include <iostream>
#include <udt.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <malloc.h>
#include <string.h>

#include "../inc/ffsnetCPP.h"
#include "../inc/ffsnet.h"

int failure = -1;

using namespace std;

int * Transfer_init(UDTArray * SsocksP, struct metadata * meta, int operation){

		/* use this function to initialize the UDT library */
		UDT::startup();

		/* allocate the struct and create the sockets */
		UDTArray Ssocks = (struct UDTArray_s *) malloc(sizeof(struct UDTArray_s));
		*SsocksP = Ssocks;
		Ssocks->socks = (UDTSOCKET *) malloc(sizeof(UDTSOCKET)*(meta->k+meta->m));
		
		Ssocks->meta = meta;
		
		int socksNumber = meta->k + meta->m;
		Ssocks->operation = operation;
		int socksreqNumber;
		switch(operation){
			case CLIENT_SENDBUF:
				socksreqNumber = meta->k + meta->m;
				break;
			case CLIENT_RECVBUF:
				socksreqNumber = meta->k;
				break;
			default:
				socksreqNumber = meta->k + meta->m;
				break;
		}
		
		int * indexArray = (int *) malloc(sizeof(int)*socksreqNumber);
		int curIndex = 0;
		
		/* Calculate buffer numbers */
		int buffernumbers = (int)meta->fileSize / (meta->bufsize*meta->k);
		if((int)meta->fileSize % (meta->bufsize*meta->k) != 0) buffernumbers++;

		/* create sockets */
		struct addrinfo hints, *peer;
		//bool blocking = true;

		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_flags = AI_PASSIVE;
		hints.ai_family = AF_INET;
<<<<<<< HEAD
<<<<<<< HEAD
		//hints.ai_socktype = SOCK_STREAM;
		hints.ai_socktype = SOCK_DGRAM;

		bool blocking = true; // set to true if you want blocking calls
=======
		hints.ai_socktype = SOCK_STREAM;
>>>>>>> e111038... Sending Stream - Big Files issues -
=======
		hints.ai_socktype = SOCK_STREAM;
>>>>>>> e111038... Sending Stream - Big Files issues -

		UDTSOCKET fhandle;
		struct comTransfer * current = meta->loc->transfers;
		
		int i,fileAvailable;
		int everythingOK = 0; //yes by default
		char port_str[6];
		for(i=0; i < socksNumber; i++){			

			sprintf(port_str, "%d", current->port);
			
			if (0 != getaddrinfo(current->hostName, port_str, &hints, &peer)) {
				cerr << "incorrect server/peer address. " << current->hostName << ":" << port_str << endl;
				return &failure;
			}
			
			Ssocks->socks[i] = UDT::socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
			fhandle = Ssocks->socks[i];
	
			if(i < socksreqNumber){//only connect the required sockets (send = all; recv = k)
				if (UDT::ERROR == UDT::connect(Ssocks->socks[i], peer->ai_addr, peer->ai_addrlen)) {
					cerr << "connect: " << UDT::getlasterror().getErrorMessage() << endl;
					if(socksreqNumber < socksNumber-1) socksreqNumber++;//if can't connect, then make sure to get one of the emergency one
					else return &failure;
				}
				else{
					// send the request type
					int opeAddress = operation;
					if (UDT::ERROR == UDT::sendmsg(fhandle, (char*)&operation, sizeof(int), -1, true))	{
						cout << "opeSend: " << UDT::getlasterror().getErrorMessage() << endl;
						return &failure;
					}
					// send the filename length
					int len = strlen(current->distantChunkName);
					if (UDT::ERROR == UDT::sendmsg(fhandle, (char*)&len, sizeof(int), -1, true)) {
						cout << "filename len Send: " << UDT::getlasterror().getErrorMessage() << endl;
						return &failure;
					}
					// send the filename
					if (UDT::ERROR == UDT::sendmsg(fhandle, current->distantChunkName, len, -1, true)) {
						cout << "filename send: " << UDT::getlasterror().getErrorMessage() << endl;
						return &failure;
					}
					
					switch(operation){
						case CLIENT_SENDBUF:{
							/* send buffersize information */
							if (UDT::ERROR == UDT::sendmsg(fhandle, (char *) &(meta->bufsize), sizeof(int), -1, true)) {
								cout << "buffersize send: " << UDT::getlasterror().getErrorMessage() << endl;
								return &failure;
							}
							/* send buffernumbers information */
							if (UDT::ERROR == UDT::sendmsg(fhandle, (char *) &(buffernumbers), sizeof(int), -1, true)) {
								cout << "buffernumbers send: " << UDT::getlasterror().getErrorMessage() << endl;
								return &failure;
							}
							//UDT::setsockopt(fhandle, 0, UDT_SNDSYN, &blocking, sizeof(bool)); //makes the socket non-blocking
							break;
							}
						case CLIENT_RECVBUF:{
								/* Receive file status (0 = available) */
								if (UDT::ERROR == UDT::recvmsg(fhandle, (char*) &fileAvailable, sizeof(int))) {
									cout << "recv: " << UDT::getlasterror().getErrorMessage() << endl;
									return 0;   
								}
								
								if(fileAvailable == 0){
									cout << "Socket number:" << i << ";Index:" << curIndex << ";ChunkName:" << current->distantChunkName << endl;
								}
								else{
									cout << "NON AVAILABLE Socket number:" << i << ";Index:" << curIndex << ";ChunkName:" << current->distantChunkName << endl;
									if(socksreqNumber < socksNumber-1){
										socksreqNumber++;
										everythingOK=1;
									}//if can't connect, then make sure to get one of the emergency one
									else return &failure;
								}
								break;
							}
						default:
							break;
					
					}
					// Add the socket to the index (this socket is open and file is available)
					if(everythingOK == 0) indexArray[curIndex++]=i;
				}
			}
			current = current->next;
		}
		
		freeaddrinfo(peer);
		Ssocks->socknumber = ++i;
		
		
		Ssocks->indexArray = indexArray;
		return indexArray;
		

		
}
int bufferSend(UDTArray Ssocks, int index, unsigned char * buffer){
	
<<<<<<< HEAD
<<<<<<< HEAD
	if (UDT::ERROR == (res = UDT::sendmsg(Ssocks->socks[index], (char *)buffer, bufsize, -1, true))) {
		if(UDT::getlasterror().getErrorCode() != 6001){ //6001 == EASYNCSND
			cout << "Send: " << UDT::getlasterror().getErrorMessage() << endl; 
			return res;
			}
		else{
			return -1;
		}
=======
=======
>>>>>>> e111038... Sending Stream - Big Files issues -
	printf("meta->bufsize: %i",Ssocks->meta->bufsize);
	
	if (UDT::ERROR == UDT::send(Ssocks->socks[index], (char *)buffer, Ssocks->meta->bufsize, 0)) {
		cout << "Send: " << UDT::getlasterror().getErrorMessage() << endl;
		return 1;
<<<<<<< HEAD
>>>>>>> e111038... Sending Stream - Big Files issues -
=======
>>>>>>> e111038... Sending Stream - Big Files issues -
	}

	return 0;
}
int bufferRecv(UDTArray Ssocks, int index, unsigned char * buffer){
	
<<<<<<< HEAD
<<<<<<< HEAD
	int res;
	if (UDT::ERROR == (res = UDT::recvmsg(Ssocks->socks[index], (char *)buffer, bufsize))) {
		if(UDT::getlasterror().getErrorCode() != 6002){ //6001 == EASYNCSND
=======
	if (UDT::ERROR == UDT::recv(Ssocks->socks[index], (char *)buffer, Ssocks->meta->bufsize, 0)) {
>>>>>>> e111038... Sending Stream - Big Files issues -
=======
	if (UDT::ERROR == UDT::recv(Ssocks->socks[index], (char *)buffer, Ssocks->meta->bufsize, 0)) {
>>>>>>> e111038... Sending Stream - Big Files issues -
			cout << "Receive: " << UDT::getlasterror().getErrorMessage() << endl;
			return 1;
	}

	return 0;
}

int Transfer_destroy(UDTArray Ssocks){

	int resCode = 1;

	switch(Ssocks->operation){
			case CLIENT_SENDBUF:{
				/* Receive ACK */
				long totalreceived = 0;
				long buffreceived;
				for (int j = 0; j < Ssocks->meta->k + Ssocks->meta->m; j++) {
					if (UDT::ERROR == UDT::recvmsg(Ssocks->socks[j], (char *) &buffreceived, sizeof(long))) {
						cout << "buffersize send: " << UDT::getlasterror().getErrorMessage() << endl;
					}
					totalreceived+=buffreceived;
				}
				if(totalreceived < Ssocks->meta->fileSize){
					cout << "ACK NOT RELEVANT, the file may have not been well transfered" << endl;
				}
				
				break;
				}
			case CLIENT_RECVBUF:{
					for (int j = 0; j < Ssocks->meta->k; j++) {
							dbgprintf("Sending ACK to socket %i\n",j,Ssocks->indexArray[j]);
							
							if (UDT::ERROR == UDT::sendmsg(Ssocks->socks[Ssocks->indexArray[j]], (char *)&resCode, sizeof(int), -1, true)) {
								cout << "Send: " << UDT::getlasterror().getErrorMessage() << endl; 
								exit(1);//fail!	
							}
					}
				break;
				}
			default:
				break;
	
	}

	
	
	int i;
	for(i=0; i < Ssocks->socknumber; i++){
		UDT::close(Ssocks->socks[i]);
		cout << "Closed socket" << i << endl;
	}
	
	free(Ssocks->indexArray);
	free(Ssocks);
	
	/* use this function to release the UDT library */
	UDT::cleanup();	
	
	return 0;
}
