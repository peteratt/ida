/*
 * Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * Author:  Pedro Alvarez-Tabio
 * Email:   palvare3@iit.edu
 *
 */

/*
 * Interface for library calls to EC
 */
#ifndef EC_H_
#define EC_H_ 
 
#include "globals.h"
#include <ecwrapper.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

struct comLocations {
	int locationsNumber;
	struct comTransfer * transfers;
};

struct comTransfer {
	char * hostName;
	int port;
	char * distantChunkName;
	char * localChunkName;
	struct comTransfer * next;
};

struct metadata {
	char *filename;
	int k;
	int m;
	int encodingLib;
	long fileSize;
	int bufsize;
};

int ida_init(char* neighbors, char* config);
int ida_finalize();


struct metadata* ecFileEncode(char *filename, int k, int m, int bufsize, int libraryId);
int ecFileDecode(char *filename);
int ecFileSend(char *filename, int k, int m, struct comLocations * loc);
int ecFileReceive(char *filename, int k, int m, struct comLocations * loc);
int ecInsertMetadata(struct metadata* meta);
void free_struct_comLocations(struct comLocations * loc);
int getRecvLocations(char * filehash, struct comLocations * loc, int minimum);
int getSendLocations(char * filehash, struct comLocations * loc, int minimum);

#endif // EC_H_
