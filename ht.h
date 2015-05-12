#ifndef ht_h
#define ht_h

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct ht_s ht_t;

// create a new hash table with max key size and number of buckets
ht_t *
ht_new(size_t max, uint32_t size);

// set key to value, when cleanup is true, both the key and the value
// will be freed when overwritten or on a call to ht_free
// takes ownership of key and value
int
ht_set(ht_t *t, char *key, void *value, bool cleanup);

void *
ht_get(ht_t *t, const char *key);

int
ht_delete(ht_t *t, const char *key);

void
ht_free(ht_t *t);

#endif