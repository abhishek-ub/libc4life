#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "col.h"
#include "fld.h"
#include "rec.h"
#include "seqs/pair.h"
#include "seqs/slab.h"
#include "val.h"

C4STATIC(c4ls, c4rec_ts);

struct c4rec_t *c4rec_t_init(struct c4rec_t *self, size_t size) {
  self->size = size;
  c4bmap_init(&self->flds, sizeof(c4fld_t), sizeof(struct c4fld), c4fld_t_cmp);
  c4ls_prepend(c4rec_ts(), &self->ts_node);
  return self;
}

void c4rec_t_free(struct c4rec_t *self) {
  C4BMAP_DO(&self->flds, it) { c4fld_free(c4pair_right(it)); }
  c4bmap_free(&self->flds);
  c4ls_delete(&self->ts_node);
}

struct c4fld *c4rec_fld(struct c4rec_t *self,
			const char *name,
			c4fld_t offs,
			struct c4val_t *type) {
  struct c4pair *it = c4bmap_add(&self->flds, &offs);
  *(c4fld_t *)c4pair_left(it) = offs;
  return c4fld_init((struct c4fld *)c4pair_right(it), name, offs, type);
}

struct c4fld *c4rec_int32(struct c4rec_t *self, const char *name, c4fld_t offs) {
  return c4rec_fld(self, name, offs, &c4int32);
}

struct c4fld *c4rec_str(struct c4rec_t *self, const char *name, c4fld_t offs) {
  return c4rec_fld(self, name, offs, &c4str);
}

struct c4rec *c4rec_init(struct c4rec *self, struct c4rec_t *type) {
  self->type = type;
  return self;
}

void c4rec_free(struct c4rec *self) {
  C4BMAP_DO(&self->type->flds, it) {
    struct c4fld *fld = (struct c4fld *)c4pair_right(it); 
    c4val_free(fld->type, (void *)self + fld->offs);
  }
}

void c4rec_clone(struct c4rec *self) {
  C4BMAP_DO(&self->type->flds, it) {
    struct c4fld *fld = (struct c4fld *)c4pair_right(it); 
    c4val_clone(fld->type, (void *)self + fld->offs);
  }
}

int c4rec_cmp(struct c4rec *self, struct c4rec *other) {
  assert(other->type == self->type);
  
  C4BMAP_DO(&self->type->flds, it) {
    struct c4fld *fld = (struct c4fld *)c4pair_right(it);
    int res = c4val_cmp(fld->type,
			(void *)self + fld->offs,
			(void *)other + fld->offs);
    if (res) { return res; }
  }

  return 0;
}

