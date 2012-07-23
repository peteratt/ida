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

int ecFileEncode(char *filename, int k, int m, int bufsize, int libraryId);
int ecFileDecode(char *filename);
int ecFileSend(char *filename, int k, int m);
int ecFileReceive(char *filename, int k, int m);
int ecInsertMetadata(char *neighbors, char *config);
void free_struct_comLocations(struct comLocations * loc);
int getLocations(char * filehash, struct comLocations * loc, int minimum);
