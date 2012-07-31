/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 * Author:  Pedro Alvarez-Tabio
 * Email:   palvare3@iit.edu
 *
 */
#ifndef IDA_H_
#define IDA_H_ 

int ida_send(char * filename, int k, int m, int bufsize);
int ida_download(char * filename);

#endif // IDA_H_
