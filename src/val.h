#ifndef C4VAL_H
#define C4VAL_H

#include "seqs/ls.h"

struct c4val_t {
  char *name;
  size_t size;
  struct c4ls ts_node;

  void *(*clone_val)(void *);
  int (*cmp_vals)(void *, void *);
  void (*free_val)(void *);
};

extern struct c4val_t c4int32;
extern struct c4val_t c4str;

void c4init_val_ts();
struct c4ls *c4val_ts();

struct c4val_t *c4val_t_init(struct c4val_t *self,
			     const char *name,
			     size_t size);

void c4val_t_free(struct c4val_t *self);

void *c4val_clone(struct c4val_t *type, void *val);
int c4val_cmp(struct c4val_t *type, void *left, void *right);
void c4val_free(struct c4val_t *type, void *val);

#endif
