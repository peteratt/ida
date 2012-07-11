/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * quick Example of using the wrapper
 */
 
#include <ecwrapper.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
 
ecFunctions ec;



//The program is similar to gibraltar example (/examples/benchmark.cc), but uses the wrapper instead of gibraltar.
#ifndef min_test
#define min_test 2
#endif
#ifndef max_test
#define max_test 6
#endif

#define PRINT 1

double etime() {
  /* Return time since epoch (in seconds) */
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec + 1.e-6*t.tv_usec;
}

#define time_iters(var, cmd, iters) {					\
    var = -1*etime();							\
    for (int iterations = 0; iterations < iters; iterations++) cmd;	\
    var = (var + etime()) / iters; }
	
int main() {

  //int test = ec_init_Gibraltar(&ec);
  ec_init_Gibraltar(&ec);

  int iters = 5;	
  if(PRINT) printf("%% Speed test with correctness checks\n");
  if(PRINT) printf("%% datasize is n*bufsize, or the total size of all data buffers\n");
  if(PRINT) printf("%%      n        m datasize chk_tput rec_tput\n");
  
	FILE *filePe = fopen("Encode.csv","w+");
	FILE *filePd = fopen("Decode.csv","w+");
	
	fprintf(filePe,",");
	fprintf(filePd,",");
	for(int n = min_test; n <= max_test; n++){
		fprintf(filePe,"%8i,",n);
		fprintf(filePd,"%8i,",n);
	}
	fprintf(filePe,"\n");
	fprintf(filePd,"\n");
	
  for (int m = min_test; m <= max_test; m++) {
  
	fprintf(filePe,"%8i,",m);
	fprintf(filePd,"%8i,",m);
  
    for (int n = min_test; n <= max_test; n++) {		
      double chk_time, dns_time;
      if(PRINT) printf("%8i %8i ", n, m);
      ecContext context;

      int rc = ec->init(n, m, &context);

      if (rc) {
	printf("Error:  %i\n", rc);
	exit(EXIT_FAILURE);
      }
			
      /* Allocate/define the appropriate number of buffers */
      int size = 1024*1024;
      void *data;
      ec->alloc(&data, size, &size, context);
			
      for (int i = 0; i < size * n; i++)
	((char *)data)[i] = (unsigned char)rand()%256;
	
      time_iters(chk_time, ec->generate(data, size, context), iters);

      unsigned char *backup_data = (unsigned char *)malloc(size * (n+m));
      memcpy(backup_data, data, size * (n+m));
			
      char failed[256];		
      for (int i = 0; i < n+m; i++)
	failed[i] = 0;
      for (int i = 0; i < ((m < n) ? m : n); i++) {
	int probe;
	do {
	  probe = rand() % n;
	} while (failed[probe] == 1);
	failed[probe] = 1;

	/* Destroy the buffer */
	memset((char *)data + size*probe, 0, size);
      }
      
      int buf_ids[256];
      int index = 0;
      int f_index = n;
      for (int i = 0; i < n; i++) {
	while (failed[index]) {
	  buf_ids[f_index++] = index;
	  index++;
	}
	buf_ids[i] = index;
	index++;
      }
      while (f_index != n+m) {
	buf_ids[f_index] = f_index;
	f_index++;
      }
      
      void *dense_data;
      ec->alloc((void **)&dense_data, size, &size, context);
      for (int i = 0; i < m+n; i++) {
	memcpy((unsigned char *)dense_data+i*size, 
	       (unsigned char *)data+buf_ids[i]*size, size);
      }

      int nfailed = (m < n) ? m : n;
      memset((unsigned char *)dense_data+n*size, 0, size*nfailed);
      time_iters(dns_time, ec->recover(dense_data, size, buf_ids, nfailed, context), 
		 iters);

      for (int i = 0; i < m+n; i++) {
	if (memcmp((unsigned char *)dense_data+i*size, 
		   backup_data+buf_ids[i]*size, size)) {
	  printf("Dense test failed on buffer %i/%i.\n", i, 
		 buf_ids[i]);
	  exit(1);
	}
      }
						
      double size_mb = size * n / 1024.0 / 1024.0;
      if(PRINT) printf("%8i %8.3lf %8.3lf\n", size*n, size_mb/chk_time, size_mb/dns_time);
		
		fprintf(filePe,"%8.3lf,",size_mb/chk_time);
		fprintf(filePd,"%8.3lf,",size_mb/dns_time);
			
      ec->free(data, context);
      ec->free(dense_data, context);
      free(backup_data);		
      ec->destroy(context);
    }
	fprintf(filePe,"\n");
	fprintf(filePd,"\n");
  }
  
  fclose(filePe);
	fclose(filePd);
  return 0;
}
