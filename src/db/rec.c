#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "col.h"
#include "fld.h"
#include "rec.h"
#include "seqs/pair.h"
#include "seqs/slab.h"
#include "val.h"

struct c4rec_t *c4rec_t_init(struct c4rec_t *self) {
  c4bmap_init(&self->flds, sizeof(c4fld_t), sizeof(struct c4fld), c4fld_t_cmp);
  return self;
}

void c4rec_t_free(struct c4rec_t *self) {
  C4BMAP_DO(&self->flds, it) { c4fld_free(c4pair_right(it)); }
  c4bmap_free(&self->flds);
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

struct c4rec *c4rec_init(struct c4rec *self, struct c4rec_t *type, c4uid_t id) {
  self->type = type;
  if (id) { c4uid_copy(self->id, id); }
  else { c4uid_init(self->id); }
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
    void *ptr = (void *)self + fld->offs;
    *(void **)ptr = c4val_clone(fld->type, ptr);  
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

