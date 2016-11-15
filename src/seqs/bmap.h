#ifndef C4BMAP_H
#define C4BMAP_H

#include <stdbool.h>
#include <stddef.h>
#include "bset.h"
#include "seq.h"

#define C4BMAP(var, key_size, val_size, cmp)	\
  struct c4bmap var;				\
  c4bmap_init(&var, key_size, val_size, cmp);	\

struct c4pair;

struct c4bmap {
  c4cmp_t cmp;
  size_t key_size;
  struct c4bset its;
  void *cmp_data;
};

/*struct c4bmap_it {
  void *key, *val;
  };*/

struct c4bmap_seq {
  struct c4bset_seq seq;
};

struct c4bmap *c4bmap_init(struct c4bmap *self,
			   size_t key_size, size_t val_size,
			   c4cmp_t cmp);
void c4bmap_clear(struct c4bmap *self);
void c4bmap_free(struct c4bmap *self);
struct c4pair *c4bmap_add(struct c4bmap *self, void *key);
struct c4pair *c4bmap_find(struct c4bmap *self, void *key, size_t start,
			      size_t *idx);
void *c4bmap_get(struct c4bmap *self, void *key);
struct c4pair *c4bmap_idx(struct c4bmap *self, size_t idx);
struct c4pair *c4bmap_insert(struct c4bmap *bmap, size_t idx);
void c4bmap_merge(struct c4bmap *self, struct c4bmap *src);
struct c4pair *c4bmap_set(struct c4bmap *self, void *key);
struct c4seq *c4bmap_seq(struct c4bmap *self, struct c4bmap_seq *seq);

#endif
