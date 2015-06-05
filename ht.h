#ifndef ht_h
#define ht_h

#include <stdint.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct ht_s ht_t;

// create a new hash table with max key size and number of buckets
ht_t *
ht_new(size_t max, uint32_t size);

// set key to value, when cleanup is non null, both the key and the value
// will be freed when overwritten or on a call to ht_free
// takes ownership of key and value
int
ht_set(ht_t *t, char *key, void *value, void (*cleanup)(void *node));

void *
ht_get(ht_t *t, const char *key);

int
ht_delete(ht_t *t, const char *key);

void
ht_free(ht_t *t);


#ifdef __cplusplus
}
#endif

#endif