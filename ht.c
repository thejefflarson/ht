#include "ht.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct hn_s {
  struct hn_s *next;
  char *key;
  void *value;
  bool cleanup;
} *hn_t;

struct ht_s {
  int prime;
  int max;
  hn_t table;
};

/* djb2 hash */
static uint32_t
hash(char *s) {
  uint32_t h = 5381;
  int c;

  while((c = *s++))
    h = ((h << 5) + h) + c;

  return h;
}

ht_t*
ht_new(int prime, int max){
  ht_t* h = (ht_t*) calloc(1, sizeof(ht_t));
  if(!h) return NULL;
  if(!(h->table = (hn_t) calloc(prime, sizeof(struct hn_s)))){
    free(h);
    return NULL;
  }
  h->prime = prime;
  h->max = max;
  return h;
}

static hn_t
_ht_get_node(ht_t *t, int i, char *key){
  hn_t n;
  for(n = &t->table[i]; n != NULL; n = n->next)
    if(n->key != NULL && strncmp(n->key, key, t->max) == 0)
      break;
  return n;
}

int
ht_set(ht_t *t, char *key, void *value, bool cleanup){
  int i = hash(key) % t->prime;
  hn_t n = _ht_get_node(t, i, key);

  if(n == NULL) {
    n = (hn_t) calloc(1, sizeof(hn_t));
    if(!n) return 1;
    n->next = t->table[i].next;
    t->table[i].next = n;
  }

  if(n->cleanup){
    free(n->key);
    free(n->value);
  }

  n->key     = key;
  n->value   = value;
  n->cleanup = cleanup;

  return 0;
}

void *
ht_get(ht_t *t, char *key){
  int i = hash(key) % t->prime;

  hn_t n = _ht_get_node(t, i, key);

  if(n) return n->value;

  return NULL;
}

void
ht_free(ht_t* t){
  hn_t f;

  for(int i = 0; i < t->prime; i++) {
    // skip the first element which we'll free later
    for(hn_t n = t->table[i].next; n != NULL;) {
      f = n->next;
      if(n->cleanup){
        free(n->key);
        free(n->value);
      }
      free(n);
      n = f;
    }
  }

  free(t->table);
  free(t);
}

