/**
 * File: ffsnet_bridger.cpp
 * Desc: This is a C wrapper to call the ffsnet library
 * Author: dzhao8@hawk.iit.edu
 * History:
 * 		06/25/2011 - initial development
 *
 * Compile to a shared library:
 *		g++ -fPIC ffsnet_bridger.cpp --shared -o libffsnet_bridger.so -L. -lffsnet
 *
 */

#include "../inc/zht_bridger.h"
#include "../lib/ZHT/inc/meta.pb.h"

const char* zht_parse_meta(struct metadata* meta) {
	Package package;
	package.set_virtualpath(meta->filename); //as key
	package.set_isdir(false);
	package.set_replicano(1);
	package.set_operation(3); //1 for look up, 2 for remove, 3 for insert
	package.set_realfullpath(meta->filename);
	
	// IDA metadata
	package.set_k(meta->k);
	package.set_m(meta->m);
	package.set_encodinglib(meta->encodingLib);
	package.set_filesize(meta->fileSize);
	package.set_bufsize(meta->bufsize);

	return package.SerializeAsString().c_str();
}

