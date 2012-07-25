#ifdef __cplusplus
extern "C" {
#endif

#include "ec.h"
#include "../lib/ZHT/inc/c_zhtclientStd.h"
  
const char* zht_parse_meta(struct metadata* meta);
int ZHTgetLocations(ZHTClient_c zhtClient, struct comLocations * loc, int n);

#ifdef __cplusplus
}
#endif
