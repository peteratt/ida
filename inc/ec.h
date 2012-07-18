/*
 * Interface for library calls to EC
 */
#include <ecwrapper.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

int ecFileEncode(char *filename, int k, int m, int bufsize);
int ecFileDecode(char *filename);
int ecFileSend(char *filename, int k, int m);
int ecFileReceive(char *filename, int k, int m);
int ecInsertMetadata(char *neighbors, char *config);

