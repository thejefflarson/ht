#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ht.h"

typedef struct hn_s {
  struct hn_s *next;
  char *key;
  void *value;
  bool cleanup;
} *hn_t;

struct ht_s {
  uint64_t ne;
  uint64_t nb;
  size_t max;
  hn_t table;
};

#define KEY_LEN 16
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
uint8_t hkey[KEY_LEN] = {0};
bool key_init = false;

static void
init_key();

static uint64_t
siphash(const uint8_t *in, uint64_t inlen, const uint8_t *k);

static hn_t
get_node(ht_t *t, int i, const char *key) {
  hn_t n = NULL;
  for(n = &t->table[i]; n != NULL; n = n->next)
    if(n->key != NULL && strncmp(n->key, key, t->max) == 0)
      break;
  return n;
}

static void
free_node(hn_t n) {
  if(n->cleanup){
    free(n->key);
    free(n->value);
  }
  n->key = NULL;
  n->value = NULL;
}

ht_t *
ht_new(size_t max, uint32_t size) {
  ht_t* h = (ht_t*) calloc(1, sizeof(ht_t));
  h->table = (hn_t) calloc(size, sizeof(struct hn_s));
  h->ne = 0;
  h->nb = size;
  h->max = max;
  return h;
}

int
ht_set(ht_t *t, char *key, void *value, bool cleanup) {
  init_key();
  uint32_t i = siphash((const uint8_t *)key, strnlen(key, t->max), hkey) % t->nb;
  hn_t n = get_node(t, i, key);

  if(n == NULL) {
    t->ne++;
    if(t->table[i].key != NULL) {
      n = (hn_t) calloc(1, sizeof(struct hn_s));
      n->next = t->table[i].next;
      t->table[i].next = n;
    } else {
      n = &t->table[i];
    }
  }

  if(n->cleanup){
    free(n->key);
    free(n->value);
  }

  n->key     = key;
  n->value   = value;
  n->cleanup = cleanup;

  if((double)t->ne / (double)t->nb > 0.80 && t->nb <= pow(2, 32) / 2) {
    uint32_t ns = t->nb << 2;
    ht_t *nt = ht_new(t->max, ns);
    for(uint32_t i = 0; i < t->nb; i++) {
      if(t->table[i].key != NULL)
        ht_set(nt, t->table[i].key, t->table[i].value, t->table[i].cleanup);
      hn_t f;
      for(hn_t n = t->table[i].next; n != NULL;) {
        f = n->next;
        ht_set(nt, n->key, n->value, n->cleanup);
        free(n);
        n = f;
      }
    }

    free(t->table);
    t->table = nt->table;
    t->nb = ns;
    free(nt);
  }

  return 0;
}

void *
ht_get(ht_t *t, const char *key) {
  uint32_t i = siphash((const uint8_t *)key, strnlen(key, t->max), hkey) % t->nb;
  hn_t n = get_node(t, i, key);
  if(n != NULL) return n->value;
  return NULL;
}

int
ht_delete(ht_t *t, const char *key) {
  uint32_t i = siphash((const uint8_t *)key, strnlen(key, t->max), hkey) % t->nb;
  hn_t n = get_node(t, i, key);
  hn_t p = NULL;

  for(hn_t c = &t->table[i]; c != NULL; p = c, c = c->next) {
    if(c == n) {
      free_node(c);
      if(p != NULL) {
        p->next = c->next;
        free(c);
      }
      t->ne--;
      return 0;
    }
  }

  return -1;
}

void
ht_free(ht_t *t) {
  hn_t f;

  for(uint32_t i = 0; i < t->nb; i++) {
    for(hn_t n = t->table[i].next; n != NULL;) {
      f = n->next;
      free_node(n);
      free(n);
      n = f;
    }
    free_node(&t->table[i]);
  }

  free(t->table);
  free(t);
}

static void
init_key() {
  pthread_mutex_lock(&lock);
  if(key_init == true) {
    pthread_mutex_unlock(&lock);
    return;
  }

  FILE *rnd;

  for(;;) {
    rnd = fopen("/dev/urandom", "r");
    if (rnd != NULL) break;
  }

  fread(hkey, sizeof(uint8_t), KEY_LEN, rnd);
  fclose(rnd);
  key_init = true;
  pthread_mutex_unlock(&lock);
}

/* default: SipHash-2-4 */
#define cROUNDS 2
#define dROUNDS 4

#define ROTL(x,b) (uint64_t)( ((x) << (b)) | ( (x) >> (64 - (b))) )

#define U32TO8_LE(p, v)                                         \
  (p)[0] = (uint8_t)((v)      ); (p)[1] = (uint8_t)((v) >>  8); \
  (p)[2] = (uint8_t)((v) >> 16); (p)[3] = (uint8_t)((v) >> 24);

#define U64TO8_LE(p, v)                        \
  U32TO8_LE((p),     (uint32_t)((v)      ));   \
  U32TO8_LE((p) + 4, (uint32_t)((v) >> 32));

#define U8TO64_LE(p)            \
  (((uint64_t)((p)[0])      ) | \
   ((uint64_t)((p)[1]) <<  8) | \
   ((uint64_t)((p)[2]) << 16) | \
   ((uint64_t)((p)[3]) << 24) | \
   ((uint64_t)((p)[4]) << 32) | \
   ((uint64_t)((p)[5]) << 40) | \
   ((uint64_t)((p)[6]) << 48) | \
   ((uint64_t)((p)[7]) << 56))

#define SIPROUND                                        \
  do {                                                  \
    v0 += v1; v1=ROTL(v1,13); v1 ^= v0; v0=ROTL(v0,32); \
    v2 += v3; v3=ROTL(v3,16); v3 ^= v2;                 \
    v0 += v3; v3=ROTL(v3,21); v3 ^= v0;                 \
    v2 += v1; v1=ROTL(v1,17); v1 ^= v2; v2=ROTL(v2,32); \
  } while(0)

static uint64_t
siphash(const uint8_t *in, uint64_t inlen, const uint8_t *k) {
  /* "somepseudorandomlygeneratedbytes" */
  uint64_t v0 = 0x736f6d6570736575ULL;
  uint64_t v1 = 0x646f72616e646f6dULL;
  uint64_t v2 = 0x6c7967656e657261ULL;
  uint64_t v3 = 0x7465646279746573ULL;
  uint64_t b;
  uint64_t k0 = U8TO64_LE( k );
  uint64_t k1 = U8TO64_LE( k + 8 );
  uint64_t m;
  int i;
  const uint8_t *end = in + inlen - (inlen % sizeof(uint64_t));
  const int left = inlen & 7;
  b = ((uint64_t) inlen) << 56;
  v3 ^= k1;
  v2 ^= k0;
  v1 ^= k1;
  v0 ^= k0;

  for (; in != end; in += 8)
  {
    m = U8TO64_LE(in);
    v3 ^= m;
    for(i = 0; i < cROUNDS; ++i) SIPROUND;
    v0 ^= m;
  }

  switch(left) {
    case 7: b |= ((uint64_t) in[ 6])  << 48;
    case 6: b |= ((uint64_t) in[ 5])  << 40;
    case 5: b |= ((uint64_t) in[ 4])  << 32;
    case 4: b |= ((uint64_t) in[ 3])  << 24;
    case 3: b |= ((uint64_t) in[ 2])  << 16;
    case 2: b |= ((uint64_t) in[ 1])  <<  8;
    case 1: b |= ((uint64_t) in[ 0]); break;
    case 0: break;
  }

  v3 ^= b;
  for(i = 0; i < cROUNDS; ++i) SIPROUND;
  v0 ^= b;
  v2 ^= 0xff;
  for(i=0; i < dROUNDS; ++i) SIPROUND;
  return v0 ^ v1 ^ v2  ^ v3;
}
