#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "fld.h"

int c4fld_cmp(void *_x, void *_y, void *data) {
  struct c4fld *x = _x, *y = _y;
  return c4fld_t_cmp(&x->offs, &y->offs, data);
}

int c4fld_t_cmp(void *_x, void *_y, void *data) {
  c4fld_t *x = _x, *y = _y;
  if (*x < *y) { return -1; }
  return *x > *y;
}

struct c4fld *c4fld_init(struct c4fld *self,
			 const char *name,
			 c4fld_t offs,
			 struct c4val_t *type) {
  self->name = strdup(name);
  self->offs = offs;
  self->type = type;
  return self;
}

void c4fld_free(struct c4fld *self) { free(self->name); }
