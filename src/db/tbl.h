#ifndef C4TBL_H
#define C4TBL_H

#include "seqs/bset.h"
#include "seqs/seq.h"

#define C4TBL(var, name, rec_t)			\
  struct c4tbl var;				\
  c4tbl_init(&var, name, rec_t)			\

struct c4rec;
struct c4rec_t;

struct c4tbl {
  char *name;
  struct c4bset recs;
  struct c4rec_t *rec_t;
};

struct c4tbl_seq {
  struct c4seq seq;
  struct c4bset_seq recs_seq;
};

struct c4tbl *c4tbl_init(struct c4tbl *self,
			 const char *name,
			 struct c4rec_t *rec_t);
void c4tbl_free(struct c4tbl *self);
struct c4seq *c4tbl_seq(struct c4tbl *self, struct c4tbl_seq *seq);
struct c4rec *c4tbl_set(struct c4tbl *self, c4uid_t id);

#endif
