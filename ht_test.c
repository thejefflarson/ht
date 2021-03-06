#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ht.h"

int
main(){
  ht_t *h = ht_new(20, 2);

  const char *k = "yo";
  const char *v = "there";
  ht_set(h, (char *)k, (void *)v, NULL);
  assert(v == ht_get(h, k));

  const char *k2 = "yo";
  const char *v2 = "hello";
  ht_set(h, (char *)k2, (void *)v2, NULL);
  assert(v2 == ht_get(h, k2));

  const char *k3 = "caveat";
  const char *v3 = "emptor";
  ht_set(h, (char *)k3, (void *)v3, NULL);
  assert(v3 == ht_get(h, k3));

  ht_delete(h, k3);
  assert(NULL == ht_get(h, k3));

  clock_t t = clock();
  for(int i = 1; i < 256; i++) {
    uint32_t *k = (uint32_t *)calloc(1, sizeof(uint32_t) + 1);
    *k = i;
    uint32_t *v = (uint32_t *)calloc(1, sizeof(uint32_t) + 1);
    *v = i;
    ht_set(h, (char *)k, (void *)v, free);
    assert(v == ht_get(h, (char *)k));
  }
  printf("%f\n", (double) (clock() - t) / CLOCKS_PER_SEC);

  t = clock();
  for(int i = 1; i < 256; i++) {
    uint32_t *v = (uint32_t *)ht_get(h, (char *)&i);
    assert(i == *v);
  }
  printf("%f\n", (double) (clock() - t) / CLOCKS_PER_SEC);


  ht_free(h);
  return 0;
}