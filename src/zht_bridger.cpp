/**
 * File: zht_bridger.cpp
 * Desc: This is a C wrapper to call the ZHT Protocol Buffers library
 * Author: palvare3@hawk.iit.edu
 * Author: cdebains@hawk.iit.edu
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
	
	std::string serialized = package.SerializeAsString();
	
	return serialized.c_str();
}

struct metadata* zht_unparse_meta(const char* text) {

	Package package;
	package.ParseFromString(serialized);
	
	struct metadata* meta;
	
	metadata->filename = package.filename();
	metadata->k = package.k();
	metadata->m = package.m();
	metadata->bufsize = package.bufsize();
	metadata->fileSize = package.filesize();
	metadata->encodingLib = package.encodinglib();
	
	return meta;
}

