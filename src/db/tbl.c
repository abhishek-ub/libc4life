#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "col.h"
#include "coro.h"
#include "macros.h"
#include "rec.h"
#include "seqs/pair.h"
#include "tbl.h"
#include "uid.h"

static void *get_id(void *it) { return ((struct c4rec *)it)->id; }

struct c4tbl *c4tbl_init(struct c4tbl *self, 
			 const char *name,
			 struct c4rec_t *rec_t) {
  self->name = strdup(name);
  self->rec_t = rec_t;
  c4bset_init(&self->recs, rec_t->size, c4uids_cmp);
  self->recs.get_key = get_id;
  return self;
}

void c4tbl_free(struct c4tbl *self) {
  free(self->name);
  C4BSET_DO(&self->recs, rec) { c4rec_free(rec); }
  c4bset_free(&self->recs);
}

static void *seq_next(struct c4seq *_seq) {
  struct c4tbl_seq *seq = C4PTROF(c4tbl_seq, seq, _seq);
  struct c4seq *recs_seq = &seq->recs_seq.seq;
  return c4seq_next(recs_seq);
}

struct c4seq *c4tbl_seq(struct c4tbl *self, struct c4tbl_seq *seq) {
  c4seq_init(&seq->seq);
  seq->seq.next = seq_next;
  c4bset_seq(&self->recs, &seq->recs_seq);
  return &seq->seq;
}

struct c4rec *c4tbl_set(struct c4tbl *self, c4uid_t _id) {
  size_t idx;
  c4uid_t id;
  if (_id) { c4uid_copy(id, _id); }
  else { c4uid_init(id); }
  struct c4rec *rec = c4bset_find(&self->recs, id, 0, &idx);
  if (rec) { return rec; }

  rec = c4bset_insert(&self->recs, idx);
  c4rec_init(rec, self->rec_t);
  c4uid_copy(rec->id, id);
  return rec;
}
