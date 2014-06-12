#include "ht.h"
#include <stdio.h>
#include <assert.h>

int
main(){
  ht_t h = ht_new(27, 1024);
  const char *k = "yo";
  const char *v = "there";
  ht_set(h, (char *)k, (void*)v, false);
  assert(v == ht_get(h, (char*)k));
  return 0;
}