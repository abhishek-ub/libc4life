#ifndef C4_H
#define C4_H

#include <stddef.h>

struct c4ctx;
struct c4err_t;
struct c4malloc;

extern struct c4err_t c4err;
extern struct c4malloc c4malloc;

struct c4ctx *c4ctx();
void c4init();
void c4free();

void *c4acquire(size_t size);
void c4release(void *ptr);

#endif
