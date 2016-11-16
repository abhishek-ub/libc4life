#ifndef C4SEQ_H
#define C4SEQ_H

#include <stdbool.h>

#define C4IT(type, ptr)				\
  *(type *)(ptr)				\

#define _C4SEQ(type, model, var, _tseq)		\
  struct type _tseq;				\
  struct c4seq *var = type(model, &_tseq);	\

#define C4SEQ(type, model, var)				\
  _C4SEQ(C4SYMS(type, _seq), model, var, C4GSYM(tseq))	\

struct c4seq {
  bool eof;
  size_t idx;
  void (*free)(struct c4seq *);
  void *(*next)(struct c4seq *);
};

struct c4seq *c4seq_init(struct c4seq *self);
void c4seq_free(struct c4seq *self);
void *c4seq_next(struct c4seq *self);

#endif
