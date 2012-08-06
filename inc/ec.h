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

struct comLocations {
	int locationsNumber;
	struct comTransfer * transfers;
};

struct comTransfer {
	char * hostName;
	int port;
	int chunkNumber;
	char * distantChunkName;
	char * localChunkName;
	struct comTransfer * next;
};

struct metadata {
	const char *filename;
	int k;
	int m;
	int encodingLib;
	long fileSize;
	int bufsize;
	struct comLocations* loc;
};

int ida_init(char* neighbors, char* config);
int ida_finalize();


struct metadata* ecFileEncode(char *filepath, int k, int m, int bufsize, int libraryId);
int ecFileDecode(char *filepath, struct metadata * meta);
int ecFileSend(char *filepath, int k, int m, struct comLocations * loc);
int ecFileReceive(char *filepath, int k, int m, struct comLocations * loc);
int ecInsertMetadata(struct metadata* meta);
struct metadata* ecLookupMetadata(char* key);
void free_struct_comLocations(struct comLocations * loc);
int getRecvLocations(char * filehash, struct comLocations * loc, int minimum);
int getSendLocations(char * filename, struct comLocations * loc, int minimum);

#endif // EC_H_
