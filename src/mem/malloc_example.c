#include <stddef.h>
#include "macros.h"
#include "malloc_example.h"

static void *acquire(struct c4malloc *_self, size_t size) {
  //struct my_malloc *self = C4PTROF(my_malloc, malloc, _self);

  // Implement your algorithm here
  
  return NULL;
}

//static void release(struct c4malloc *_self, void *ptr) {
  //struct my_malloc *self = C4PTROF(my_malloc, malloc, _self);

  // Implementing release is optional,
  // as long as all memory is deallocated with the allocator
//}

struct my_malloc *my_malloc_init(struct my_malloc *self) {
  c4malloc_init(&self->malloc);
  self->malloc.acquire = acquire;
  self->malloc.release = NULL; //release;

  // Initialize your state here
  
  return self;
}

void my_malloc_free(struct my_malloc *self) {
  c4malloc_free(&self->malloc);

  // Free any memory allocated from the heap here
}
