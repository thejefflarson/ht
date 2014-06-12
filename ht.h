#ifndef ht_h
#define ht_h
#include <stdbool.h>

typedef struct ht_s ht_t;

// create a new hash table with prime buckets and max key size
ht_t *
ht_new(int prime, int max);

// set key to value, when cleanup is true, both the key and the value
// will be freed when overwritten or on a call to ht_free
int
ht_set(ht_t* t, char *key, void* value, bool cleanup);

void*
ht_get(ht_t* t, char *key);

void
ht_free(ht_t* t);

#endif