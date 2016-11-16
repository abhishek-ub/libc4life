#ifndef C4FLD_H
#define C4FLD_H

#include <stddef.h>

#define C4FLD(type, fld)			\
  offsetof(struct type, fld)			\

struct c4val_t;

typedef size_t c4fld_t;

struct c4fld {
  char *name;
  c4fld_t offs;
  struct c4val_t *type;
};

int c4fld_cmp(void *_x, void *_y, void *data);
int c4fld_t_cmp(void *_x, void *_y, void *data);

struct c4fld *c4fld_init(struct c4fld *self,
			 const char *name,
			 c4fld_t offs,
			 struct c4val_t *type);

void c4fld_free(struct c4fld *self);

#endif
