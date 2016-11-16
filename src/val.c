#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "seqs/seq.h"
#include "val.h"

struct c4val_t c4int32;
struct c4val_t c4str;

C4STATIC(c4ls, c4val_ts);

int c4cmp_int32(void *left, void *right) {
  int32_t l = C4IT(int32_t, left), r = C4IT(int32_t, right);
  if  (l < r) return -1;
  return l > r;
}

static void str_clone(void *val) {
  C4IT(char *, val) = strdup(C4IT(char *, val));
}

static int str_cmp(void *left, void *right) {
  return strcmp(*(char **)left, *(char **)right);
}

static void str_free(void *val) { free(*(char **)val); }

void c4init_val_ts() {
  c4val_t_init(&c4int32, "int32", sizeof(int32_t));
  c4int32.cmp_vals = c4cmp_int32;
  
  c4val_t_init(&c4str, "str", sizeof(char *));
  c4str.clone_val = str_clone;
  c4str.cmp_vals = str_cmp;
  c4str.free_val = str_free;
}

struct c4val_t *c4val_t_init(struct c4val_t *self,
			     const char *name,
			     size_t size) {
  self->name = strdup(name);
  self->size = size;
  c4ls_prepend(c4val_ts(), &self->ts_node);
  self->clone_val = NULL;
  self->free_val = NULL;
  return self;
}

void c4val_t_free(struct c4val_t *self) {
  free(self->name);
  c4ls_delete(&self->ts_node);
}

void c4val_clone(struct c4val_t *type, void *val) {
  if (type->clone_val) { type->clone_val(val); }
}

int c4val_cmp(struct c4val_t *type, void *left, void *right) {
  assert(type->cmp_vals);
  return type->cmp_vals(left, right);
}

void c4val_free(struct c4val_t *type, void *val) {
  if (type->free_val) { type->free_val(val); }
}
