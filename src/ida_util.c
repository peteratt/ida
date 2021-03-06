/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * Author:  Pedro Alvarez-Tabio
 * Email:   palvare3@iit.edu
 *
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
 
int randomStr(char * destination, int destLen){
	char * password_chars = "1234567890abcdefghijklmnopqrstuvwxyz";
	unsigned int iseed = (unsigned int)time(NULL) ^ (getpid()<<16);
 	srand (iseed);
	int i;
	int random;

	for (i = 0; i < destLen; i++) {
		random = rand()%(strlen(password_chars)-1);
		destination[i] = password_chars[random];
	}
	destination[destLen-1] = '\0';
	
	return 0;
}

char* get_filename_from_path(char* filepath){
 
	char *filename;
	filename = strrchr(filepath, '/');

	if(filename == NULL) 
		return filepath;
	else 
		return filename+1;
}

int getBufferNumbers(int filesize,int k , int bufsize){
		int buffernumbers = filesize / (bufsize*k);
		if(filesize % (bufsize*k) != 0) buffernumbers++;
		
		return buffernumbers;
}


int getchunksize(int filesize, int k, int bufsize){
	//k is the number of data chunks in EC
	//filesize is the size of a file
	// bufsize is the size of a buffer in B
	
	// returns the size of a chunk

	return getBufferNumbers(filesize,k , bufsize)*bufsize;

}

