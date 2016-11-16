#ifndef C4REC_H
#define C4REC_H

#include "fld.h"
#include "seqs/bmap.h"
#include "seqs/ls.h"
#include "uid.h"

#define C4REC_CLONE(dest, src)			\
  { *(dest) = *(src);				\
    c4rec_clone((struct c4rec *)dest); }	\

#define C4REC_T(var)				\
  struct c4rec_t var;				\
  c4rec_t_init(&var);				\

struct c4rec_t {
  struct c4bmap flds;
};

struct c4ls *c4rec_ts();

struct c4rec_t *c4rec_t_init(struct c4rec_t *self);
void c4rec_t_free(struct c4rec_t *self);

struct c4fld *c4rec_fld(struct c4rec_t *self,
			const char *name,
			c4fld_t fld,
			struct c4val_t *type);

struct c4fld *c4rec_int32(struct c4rec_t *self, const char *name, c4fld_t offs);
struct c4fld *c4rec_str(struct c4rec_t *self, const char *name, c4fld_t offs);

struct c4rec {
  struct c4rec_t *type;
  c4uid_t id;
};

struct c4rec *c4rec_init(struct c4rec *self, struct c4rec_t *type, c4uid_t id);
void c4rec_free(struct c4rec *self);

void c4rec_clone(struct c4rec *self);
int c4rec_cmp(struct c4rec *self, struct c4rec *other);

#endif
