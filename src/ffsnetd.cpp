/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 *
 */

#include <cstdlib>
#include <netdb.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <udt.h>
#include <malloc.h>

#include <linux/limits.h>
#include <sys/stat.h>
#include <cerrno>

#include "ffsnet.h"

using namespace std;

/* RECEIVE FILE */
int recvFile(UDTSOCKET * fhandleP, char * filepath){
		
		UDTSOCKET fhandle = *fhandleP;
		
		/* open the file to write */
		fstream fileS(filepath, ios::out | ios::binary | ios::trunc);
		int buffersize;
		int bufnumber; 
		int64_t offset = 0;
		int64_t recvsize;		
		
		/* get buffersize information */
		if (UDT::ERROR == UDT::recv(fhandle, (char*)&buffersize, sizeof(int), 0)) {

			UDT::close(fhandle);
			fileS.close();

			cout << "Buffersize Reception ERROR: " << UDT::getlasterror().getErrorMessage() << endl;
			return 1;
		}
		printf("recv buffersize:%i \n",buffersize);
		
		/* get buffernumbers information */
		if (UDT::ERROR == UDT::recv(fhandle, (char*)&bufnumber, sizeof(int), 0)) {

			UDT::close(fhandle);
			fileS.close();

			cout << "buffernumber Reception ERROR: " << UDT::getlasterror().getErrorMessage() << endl;
			return 1;
		}
		printf("recv buffernumber:%i \n",bufnumber);

		if (buffersize < 0 || bufnumber < 0) {

			UDT::close(fhandle);
			fileS.close();

			cout << "BufferSize or BufferNumber inconsistent \n";
			return 1;
		}

		cout << "Now Waiting for data \n";

		/* receive the buffers */
		int bufGroupSize = 1; //currently we receive 4 buffers before writing
		char * buffer = (char *) malloc(sizeof(char)*bufGroupSize*buffersize);
		long res = 0;
		
		int bufGroupNumber = bufnumber/bufGroupSize;
		if(bufnumber % bufGroupSize != 0) bufGroupNumber++;
		
		for(int j=0; j < bufGroupNumber; j++){
			cout << "Now Waiting for data group(" << j <<")" << endl;
			recvsize = UDT::recv(fhandle, buffer, sizeof(char)*bufGroupSize*buffersize, 0);
			if (UDT::ERROR == recvsize){
				UDT::close(fhandle);
				fileS.close();

				cout << "recvfile: " << UDT::getlasterror().getErrorMessage() << endl;
				return 1;
			}
			else{
				cout << "Writing to file " << recvsize << " Bytes" << endl;
				fileS.write(buffer,recvsize);
				res += recvsize;
			}
		}
		
		/* Send the number of bytes read as an ack */
		cout << "Sending ACK with "<< res << "Bytes" << endl;
		if (UDT::ERROR == UDT::send(fhandle, (char *)&res, sizeof(long), 0)) {
			cout << "Send: " << UDT::getlasterror().getErrorMessage() << endl;
			return 1;
		}

		cout << "Closing File" << endl;
		fileS.close();
		return 0;
}

int sendFile(UDTSOCKET * fhandleP, char * filepath){

		UDTSOCKET fhandle = *fhandleP;

		int alive=0;
		
		/* open the file */
		fstream fileS(filepath, ios::in | ios::binary);
		
		if(fileS.fail()){ //failbit set to 1, error opening the file
			cout << "Cannot of the file " << filepath << endl;
			alive = 1;//file is not available
		}
		
		/* We tell the client if the file is available */
		if (UDT::ERROR == UDT::send(fhandle, (char *)&alive, sizeof(int), 0)) {
				cout << "filename send: " << UDT::getlasterror().getErrorMessage() << endl;
				return 1;
		}
		
		//If file is not alive, we return
		if(alive != 0) return 1;
		
		fileS.seekg(0, ios::end);
		int64_t totalsize = fileS.tellg();
		fileS.seekg(0, ios::beg);
		
		
		UDT::TRACEINFO trace;
		UDT::perfmon(fhandle, &trace);

		/* send the file */
		int64_t offset = 0;
		cout << "Now sending the file" << endl;
		if (UDT::ERROR == UDT::sendfile(fhandle, fileS, offset, totalsize)) {

			/* DFZ: This error might be triggered if the file size is zero, which is fine. */

			UDT::close(fhandle);
			fileS.close();

			cout << "sendfile: " << UDT::getlasterror().getErrorMessage() << endl;
			return 1;
		}

		UDT::perfmon(fhandle, &trace);
		/* cout << "speed = " << trace.mbpsSendRate << "Mbits/sec" << endl; */

		fileS.close();
}

/**
 * Thread to accept file request: download or upload
 */
void* connecHandler(void* usocket)
{
	UDTSOCKET fhandle = *(UDTSOCKET*)usocket;
	delete (UDTSOCKET*)usocket;

	/* aquiring file name information from client */
	char * filepath;
	int len;
	int operationID;
	int operationRes;
	
	/* get the request type: download or upload */
	if (UDT::ERROR == UDT::recv(fhandle, (char*)&operationID, sizeof(int), 0)) {
		cout << "recv: " << UDT::getlasterror().getErrorMessage() << endl;
		return 0;   
	}
	
	/* Get the filename (or dir) length and the string */
	if (UDT::ERROR == UDT::recv(fhandle, (char*)&len, sizeof(int), 0)) {

		UDT::close(fhandle);
		cout << "recv: " << UDT::getlasterror().getErrorMessage() << endl;
		return 0;
	}
	
	filepath = (char *) malloc(sizeof(char)*len+1);
	
	if (UDT::ERROR == UDT::recv(fhandle, filepath, len, 0)) {

		UDT::close(fhandle);
		cout << "recv: " << UDT::getlasterror().getErrorMessage() << endl;
		return 0;
	}
	filepath[len] = '\0';

	/* Choose the appropriate action in function of the operation requested */
	switch(operationID){
		case SERVER_RECVFILE:
			operationRes = recvFile(&fhandle, filepath);
			break;
		case SERVER_SENDFILE:
			operationRes = sendFile(&fhandle, filepath);
			break;
		default:
			operationRes = 1;//WTF did I just read?
	}
	


	/*clean up*/
	free(filepath);
	UDT::close(fhandle);

	return NULL;
}


int main(int argc, char* argv[])
{
	/* usage: ffsd [server_port] */
	if ((2 < argc) || ((2 == argc) && (0 == atoi(argv[1])))) {
		cout << "usage: ffsd [server_port]" << endl;
		return 0;
	}

	/* use this function to initialize the UDT library */
	UDT::startup();

	addrinfo hints;
	addrinfo* res;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	string service("9000"); /* default server port */
	if (2 == argc)
		service = argv[1];

	if (0 != getaddrinfo(NULL, service.c_str(), &hints, &res)) {
		cout << "illegal port number or port is busy.\n" << endl;
		return 0;
	}

	UDTSOCKET serv = UDT::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (UDT::ERROR == UDT::bind(serv, res->ai_addr, res->ai_addrlen)) {
		cout << "bind: " << UDT::getlasterror().getErrorMessage() << endl;
		return 0;
	}

	freeaddrinfo(res);

	cout << "FusionFS file transfer is ready at port: " << service << endl;

	UDT::listen(serv, 10);

	sockaddr_storage clientaddr;
	int addrlen = sizeof(clientaddr);

	UDTSOCKET fhandle;

	while (true) {
		// Note: everything after this loop is useless. This loop should be based on a variable, and that variable modified if a sigkill or sigstop is rece
	
		if (UDT::INVALID_SOCK == (fhandle = UDT::accept(serv, (sockaddr*)&clientaddr, &addrlen))) {
			cout << "accept: " << UDT::getlasterror().getErrorMessage() << endl;
			return 0;
		}

		char clienthost[NI_MAXHOST];
		char clientservice[NI_MAXSERV];
		getnameinfo((sockaddr *)&clientaddr, addrlen, clienthost, sizeof(clienthost), clientservice, sizeof(clientservice), NI_NUMERICHOST|NI_NUMERICSERV);
		/* cout << "new connection: " << clienthost << ":" << clientservice << endl; */
		
		pthread_t filethread;
		pthread_create(&filethread, NULL, connecHandler, new UDTSOCKET(fhandle));
		pthread_detach(filethread);
	}

	UDT::close(serv);

	/* use this function to release the UDT library */
	UDT::cleanup();

	return 0;
}


