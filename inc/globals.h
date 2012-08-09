/*
 * Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * Author:  Pedro Alvarez-Tabio
 * Email:   palvare3@iit.edu
 *
 */
#ifndef IDA_GLOBALS_H_
#define IDA_GLOBALS_H_ 



//Libraries
#define GIBRALTAR 0
#define JERASURERS 1

//Defaults Parameters
#define DEFAULT_K 4
#define DEFAULT_M 2
#define DEFAULT_ECLIBRARY 1 //DO NOT SET TO A GPU LIBRARY!!
#define DEFAULT_BUFSIZE 1024
#define CACHE_DIR_NAME "ida"
#define CACHE_DIR_PATH "./"
#define CHUNKNAME_LENGTH 64


#ifdef DEBUG
	#define dbgprintf(...) fprintf(stdout, __VA_ARGS__)
#else
	#define dbgprintf(...) fprintf(stderr, __VA_ARGS__)
#endif

#endif // IDA_GLOBALS_H_ 
