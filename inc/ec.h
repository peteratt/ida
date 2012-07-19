/*
 * Interface for library calls to EC
 */
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
	char * distantchunkName;
	char * localchunkName;
	struct comLocations * next;
}

int ecFileEncode(char *filename, int k, int m, int bufsize);
int ecFileDecode(char *filename);
int ecFileSend(char *filename, int k, int m);
int ecFileReceive(char *filename, int k, int m);
int ecInsertMetadata(char *neighbors, char *config);
void free_struct_comLocations(struct comLocations * loc);
int getLocations(char * filehash, struct comLocations * loc, int minimum);
