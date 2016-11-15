#include <assert.h>
#include <stdlib.h>
#include "bmap.h"
#include "macros.h"
#include "pair.h"
#include "seq.h"

static void *get_key(void *_it) { return c4pair_left(_it); }

struct c4bmap *c4bmap_init(struct c4bmap *self,
			   size_t key_size, size_t val_size,
			   c4cmp_t cmp) {
  self->key_size = key_size;
  self->cmp = cmp;
  self->cmp_data = NULL;
  c4bset_init(&self->its, sizeof(struct c4pair) + key_size + val_size, cmp);
  self->its.get_key = get_key;
  return self;
}

void c4bmap_free(struct c4bmap *self) { c4bset_free(&self->its); }

void c4bmap_clear(struct c4bmap *self) { c4bset_clear(&self->its); }

struct c4pair *c4bmap_add(struct c4bmap *self, void *key) {
  size_t idx;
  if (c4bmap_find(self, key, 0, &idx)) { return NULL; }
  return c4bmap_insert(self, idx);
}

struct c4pair *c4bmap_find(struct c4bmap *self, void *key, size_t start,
			      size_t *idx) {
  return c4bset_find(&self->its, key, start, idx);
}

void *c4bmap_get(struct c4bmap *self, void *key) {
  struct c4pair *it = c4bmap_find(self, key, 0, NULL);
  return it ? c4pair_right(it) : NULL;
}

struct c4pair *c4bmap_idx(struct c4bmap *self, size_t idx) {
  return c4bset_idx(&self->its, idx);
}

struct c4pair *c4bmap_insert(struct c4bmap *self, size_t idx) {
  struct c4pair *it = c4bset_insert(&self->its, idx);
  it->offs = self->key_size;
  return it;
}

void c4bmap_merge(struct c4bmap *self, struct c4bmap *src) {
  c4bset_merge(&self->its, &src->its);
}

struct c4pair *c4bmap_set(struct c4bmap *self, void *key) {
  size_t idx;
  struct c4pair *it = c4bmap_find(self, key, 0, &idx);
  if (it) { return it; }
  return c4bmap_insert(self, idx);
}

struct c4seq *c4bmap_seq(struct c4bmap *self, struct c4bmap_seq *seq) {
  return c4bset_seq(&self->its, &seq->seq);
}
