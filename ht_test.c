#include "ht.h"
#include <stdio.h>
#include <assert.h>

int
main(){
  ht_t *h = ht_new(2);

  const char *k = "yo";
  const char *v = "there";
  ht_set(h, (char *)k, (void*)v, false);
  assert(v == ht_get(h, k));

  const char *k2 = "yo";
  const char *v2 = "emptor";
  ht_set(h, (char *)k2, (void*)v2, false);
  assert(v2 == ht_get(h, k2));

  const char *k3 = "caveat";
  const char *v3 = "emptor";
  ht_set(h, (char *)k3, (void*)v3, false);
  assert(v3 == ht_get(h, k3));

  ht_free(h);
  return 0;
}