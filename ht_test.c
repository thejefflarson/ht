#include "ht.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int
main(){
  ht_t *h = ht_new(20, 32);

  const char *k = "yo";
  const char *v = "there";
  ht_set(h, (char *)k, (void*)v, false);
  assert(v == ht_get(h, k));

  const char *k2 = "yo";
  const char *v2 = "hello";
  ht_set(h, (char *)k2, (void*)v2, false);
  assert(v2 == ht_get(h, k2));

  const char *k3 = "caveat";
  const char *v3 = "emptor";
  ht_set(h, (char *)k3, (void*)v3, false);
  assert(v3 == ht_get(h, k3));

  ht_delete(h, k3);
  assert(NULL == ht_get(h, k3));

  for(int i = 0; i < 500000; i++) {
    char *k;
    asprintf(&k, "%d", i);
    char *v;
    asprintf(&v, "%d", i);
    ht_set(h, k, v, true);
    assert(v == ht_get(h, k));
  }

  ht_free(h);
  return 0;
}