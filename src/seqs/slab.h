#ifndef C4SLAB_H
#define C4SLAB_H

#include <stddef.h>
#include "seq.h"

struct c4slab {
  size_t len, it_size;
  void *its;
};

struct c4slab_seq {
  struct c4seq seq;
  struct c4slab *slab;
};

struct c4slab *c4slab_init(struct c4slab *self, size_t it_size);
void c4slab_free(struct c4slab *self);
void c4slab_grow(struct c4slab *self, size_t len);
void *c4slab_idx(struct c4slab *self, size_t idx);
void c4slab_move(struct c4slab *self, size_t dest, size_t src, size_t len);
struct c4seq *c4slab_seq(struct c4slab *self, struct c4slab_seq *seq);

#endif
