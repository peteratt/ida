#ifdef __cplusplus
extern "C" {
#endif

#include "ec.h"
#include "../lib/ZHT/inc/c_zhtclientStd.h"
  
const char* zht_parse_meta(struct metadata* meta); //deprecated
int zht_insert_meta(ZHTClient_c zhtClient, struct metadata * meta);
int ZHTgetLocations(ZHTClient_c zhtClient, struct comLocations * loc);
struct metadata* zht_unparse_meta(const char* text); //deprecated
struct metadata * zht_lookup_meta(ZHTClient_c zhtClient, const char * key);

#ifdef __cplusplus
}
#endif
