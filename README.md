# libc4life
#### esoteric C essentials

### intro
c4life is a library of ideas and tools that have accumulated over 30 years of writing software for fun. I've found that given a solid foundation, which is what this library is aiming to provide; coding in C is a welcome therapy after spending years exploring various ways of pretending the hidden complexity was someone else's problem. This library is aiming for simplicity and leverage; and it makes a real effort to get there by playing on C's strengths, rather than inventing yet another buggy Lisp.

### motivation
I've learned a lot from tinkering with my magnum opus in software over the years. As I gained more experience and learned new tricks for managing the sprawling complexity, my ambitions quickly grew to offset any advantage gained. I spent decades searching for the perfect language, discovering the perfect architecture, the perfect database; to conclude that it depends, and that's about as far as that train goes. What it's really all about is attributes such as simplicity, leverage, modularity, orthogonality etc. And the only language that allows picking as many as you can chew from that list is C. 

### status
Any feature documented here can be assumed to be reasonably stable. I'll add more pleasant surprises as soon as I consider them polished enough for a wider audience. 

### setup
In any modern Debian-based distro, this should get you started:

```bash
sudo apt-get install clang cmake uuid-dev
git clone https://github.com/codr4life/libc4life.git
cd libc4life
mkdir build
cd build
cmake ..
make
./tests
sudo make install
```

### init
c4life needs to keep track of internal state for some of it's features. Calling ```c4init()``` before accessing the library is highly recommended, ```c4free()``` deallocates all internal state. A context is allocated internally on demand per thread, calling ```c4ctx_free(c4ctx())``` deallocates the context for the current thread.

### memory
c4life is designed to support and encourage stack allocation wherever possible. Most initializers and finalizers make no assumptions about how the memory pointed to was allocated, and take no responsibility for freeing memory explicitly allocated by user code.

#### allocators
c4life provides a composable, extensible allocator protocol and a set of implementations that focus on specific aspects of dynamic memory allocation. The api is designed to support exploring different options by changing a few lines of init code. The basic allocator, ```c4malloc```, simply calls malloc() and free(). A default allocator may be specified per thread by assigning ```c4ctx()->malloc``` from inside the thread.

#### pool
The pool allocator allows treating sets of separate allocations as single blocks of memory, while retaining the ability to release individual pointers. The data needed for book keeping is prefixed to each allocation and supports O(1) addition and removal without additional allocations.

```C

#include <c4life/c4.h>
#include <c4life/defer.h>
#include <c4life/mem/mpool.h>

void mpool_tests() {
  // Define and initialize with default source

  C4MPOOL(mp, &c4malloc);
  C4DEFER({ c4mpool_free(&mp); });

  const int LEN = 10;
  void *ptrs[LEN];
  
  // Allocate memory

  for (int i = 0; i < LEN; i++) {
    ptrs[i] = c4mpool_acquire(&mp, sizeof(int));
  }

  // Release pointer

  c4mpool_release(&mp, ptrs[0]);
}

```

#### slab
The slab allocator allocates memory as slabs of user defined size and keeps track of available space within them. Since it doesn't keep any information about individual allocations; the only way to release allocated memory is to free the allocator. It's useful for reducing the number of actual allocations that a downstream allocator causes.

```C

#include <c4life/c4.h>
#include <c4life/defer.h>
#include <c4life/mem/mslab.h>

void mslab_tests() {
  const int LEN = 10;

  // Define and initialize with specified slab size and default source

  C4MSLAB(ms, sizeof(int) * LEN, &c4malloc);
  C4DEFER({ c4mslab_free(&ms); });
  
  void *prev_ptr = NULL;
  for (int i = 0; i < LEN; i++) {
    // Allocate memory
    void *ptr = c4mslab_acquire(&ms, sizeof(int));

    // Make sure we're using the same block of memory
    
    assert(!prev_ptr || ptr == prev_ptr + sizeof(int));
    
    // Make sure slab offset matches our view of reality

    assert(c4mslab_it(&ms)->offs == sizeof(int) * (i+1));
    prev_ptr = ptr;
  }

  // Trigger allocation of new slab and verify offset
  
  c4mslab_acquire(&ms, 1);
  assert(c4mslab_it(&ms)->offs == 1);
}

```

#### freelist
The freelist allocator is used to recycle released pool memory, it reuses the embedded book keeping data to track released pointers.

```C

#include <c4life/c4.h>
#include <c4life/defer.h>
#include <c4life/mem/mfreel.h>

void mfreel_tests() {
  // Define and initialize with default source
  
  C4MPOOL(mp, &c4malloc);
  C4MFREEL(mf, &mp);
  C4DEFER({ c4mfreel_free(&mf); c4mpool_free(&mp); });

  const int LEN = 10;
  void *ptrs[LEN];

  for (int i = 0; i < LEN; i++) {
    // Allocate from mpool since we know freelist is empty
    
    ptrs[i] = c4mpool_acquire(&mp, sizeof(int));
  }

  // Release all memory to freelist
  
  for (int i = 0; i < LEN; i++) { c4mfreel_release(&mf, ptrs[i]); }

  for (int i = 0; i < LEN; i++) {
    // Make sure that memory is recycled by freelist
    
    assert(c4mfreel_acquire(&mf, sizeof(int)) == ptrs[i]);
  }
}

```

#### performance
The short story is that slabs are faster than the basic allocator; adding a free list to recycle pointers is typically about as fast, while reducing memory usage. The reason the pool/slab combination sticks out is most probably that the added overhead pushes the number of slab allocations, the problem disappears when a free list is added in front. I still haven't solved the mystery of why the pool allocator by itself is faster than the basic allocator; some kind of alignment effect from the prefix, maybe; all I know is it executes additional code before delegating to the basic allocator, allocates additional memory, and still manages to run faster.

### lambdas
The ```C4LAMBDA()``` macro defines anonymous nested functions.

```C

#include <c4life/macros.h>

void lambda_tests() {
  assert(C4LAMBDA({ return x*y; }, int, int x, int y)(2, 3) == 6);
}

```

### deferred actions
c4life supports two flavors of defer, both based on cleanup attributes; one for current scope and a more elaborate version for deferring to user defined scopes.

```C

#include <c4life/defer.h>

void defer_tests() {
  bool called = false;

  {
    C4DEFER({ called = true; });
    assert(!called);
  }
  
  assert(called);
}

void defer_scope_tests() {
  int called = false;
  
  C4DEFER_SCOPE(outer) {
    C4DEFER_SCOPE(inner) {
      C4DEFER_TO(outer, { called = true; });
    }
    
    assert(!called);
  }

  assert(called);
}

```

### coroutines
c4life provides coroutines in the form of a minimalistic layer of macros inspired by Duff's Device. Anything that should persist across calls needs to be declared static, global or passed as parameters; the only thing the coroutine really cares about is the current line number. Calling a coroutine is the same as calling any other function, all the details are neatly encapsulated inside.

```C

#include <c4life/coro.h>

struct coro_ctx { int i, line; };

int coro(struct coro_ctx *ctx, int foo) {
  C4CORO(&ctx->line);
  for(ctx->i = 1; ctx->i <= 10; ctx->i++) { C4CORO_RET(foo + ctx->i); }
  C4CORO_END();
  
  return -1;
}

void coro_tests() {
  struct coro_ctx ctx = {0, 0};
  for (int i = 1; i <= 10; i++) { assert(coro(&ctx, i) == i*2); }
  assert(coro(&ctx, 0) == -1);
}

```

### sequences
c4life implements several types that provide a sequence of values; linked lists, dynamic arrays, binary sets and maps, tables and more. Each of them provide a function in the form of ```struct c4seq *[type]_seq(self, seq)``` to initialize a new sequential view of self. Any memory allocated by the sequence is automatically deallocated when it reaches it's end, or manually by calling ```c4seq_free(seq)```.

#### linked lists
c4life provides double linked lists that are designed to allow embedding. They are especially useful where there is a need to keep track of a list of structs. Since links are allocated with the struct; they enable building fully stack allocated, linked sequences. And since the list is double linked, removing items is O(1). There is no separate root type, any node can be a root.

```C

#include <c4life/seqs/ls.h>

struct ls_it {
  // Links are embedded in the item
  
  struct c4ls ls;
};

void ls_splice_tests() {
  // Initialize lists

  C4LS(foo);
  C4LS(bar);

  // Add items to lists.
  // All list operations are node based,
  // prepending to root is the same as appending
  // to the list.
  
  const int MAX = 100;
  struct ls_it its[MAX];

  for (int i = 0; i < MAX/2; i++) {
    c4ls_prepend(&foo, &its[i].ls);
    c4ls_prepend(&bar, &its[i+MAX/2].ls);    
  }

  // Append all items in bar to foo by linking
  // the entire list to the end.
  
  c4ls_splice(&foo, bar.next, bar.prev);

  // Check that all items are now in foo
  
  int i = 0;  
  C4LS_DO(&foo, it) { assert(it == &its[i++].ls); }

  // bar is left untouched, and needs to be re-initialized 
  // before further use as a root

  c4ls_init(&bar);
}

```

#### dynamic arrays
c4life provides dynamic arrays with user defined item size; they're implemented as a single block of memory that is grown automatically when needed.

```C

#include <c4life/seqs/dyna.h>

void dyna_tests() {
  // Initialize arr for int sized items
  
  C4DYNA(arr, sizeof(int));
  C4DEFER({ c4dyna_free(&arr); });

  const int LEN = 10;

  // Preallocate to fit at least LEN/2 items

  c4dyna_grow(&arr, LEN/2);

  // Populate array

  for (int i = 0; i < LEN; i++) { *(int *)c4dyna_push(&arr) = 1; }
  assert(arr.len == LEN);

  // Empty array and check reverse order

  for (int i = LEN-1; i >= 0; i--) { *(int *)c4dyna_pop(&arr) = i; }
  assert(arr.len == 0);
}


```

#### binary sets
c4life's binary sets are implemented as dynamic, binary searched arrays. Like dynamic arrays, they support user defined item sizes. The constructor requires a comparison function to be used for searching, the optional field cmp_data is passed on to the function. To enable using sets as maps with embedded keys, they support specifying a function to get the key for a given item.

```C

#include <c4life/seqs/bset.h>

int cmp_int(void *_x, void *_y, void *data) {
  int x = *(int *)_x, y = *(int *)_y;
  if  (x < y) return -1;
  return x > y;
}

void bset_tests() {
  // Initialize set and populate in reverse order
  
  C4BSET(set, sizeof(int), cmp_int);
  C4DEFER({ c4bset_free(&set); });

  static int MAX = 100;

  for (int i = MAX-1; i >= 0; i--) { C4BSET_ADD(&set, int, i); }

  // Check number of items

  assert(c4bset_len(&set) == MAX);
  
  for (int i = 0; i < MAX; i++) {
    // Look up item by key

    assert(*(int *)c4bset_get(&set, &i) == i);

    // Look up item by index
    
    assert(*(int *)c4bset_idx(&set, i) == i);
  }
}

```

#### rolling your own
Hooking into the sequence framework is trivial; you need a struct named ```[type]_seq``` to hold your state and the ```c4seq``` struct; a constructor named ```[type]_seq``` to initialize the sequence; and a function that provides the next value. The framework keeps track of index and eof status.

```C

#include <c4life/seq.h>

struct my {
  // Your type
};

struct my_seq {
  struct c4seq super;

  // Additional state, if needed
};

static void seq_free(struct c4seq *_seq) {
  struct my_seq *seq = C4PTROF(my_seq, super, _seq);

  // Code that frees additional state, if needed
}

static void *seq_next(struct c4seq *_seq) {
  struct my_seq *seq = C4PTROF(my_seq, super, _seq);

  // Code that returns next value, or NULL on eof;
  // _seq->idx contains the current index
}

struct c4seq *my_seq(struct my *self, struct my_seq *seq) {
  c4seq_init(&seq->super);
  seq->super.free = seq_free; // Optional
  seq->super.next = seq_next;

  // Code that inits additional state, if needed

  return &seq->super;
}

```

### errors
c4life adds the ability to throw and catch typed errors out of band. Throwing an error doesn't unwind the stack to make sure someone is there to catch it. Errors are accumulated in the current try scope and propagated on exit; unhandled errors are printed to ```stderr``` on final exit. Catching errors scans the accumulated error queue for the specified type or one of it's sub types. Printing includes a stacktrace with try scopes, file names and line numbers.

#### types
Since c4life keeps track of error types internally, the result of creating new error types from more than one thread at a time is undefined. ```c4free()``` frees all registered types, calling ```c4err_t_free()``` manually unregisters the type.

```C

#include <c4life/err.h>

struct c4err_t custom_type;

void err_tests() {
  c4err_t_init(&custom_type, NULL, "custom"); // NULL super type
  
  C4TRY("outer") {
    struct c4err *err = NULL;
    C4TRY("inner") { err = C4THROW(&custom_type, "test throw"); }
    bool caught = false;
    
    C4CATCH(e, &custom_type) {
      assert(e == err);
      c4err_free(e); // Handle err by freeing
      caught = true;
    }

    assert(caught);

    // Make sure queue is empty, NULL matches any type
    C4CATCH(e, NULL) { assert(false); }    
  }
}

```

### license
MIT

### support
Thanks to a permanently messed up spine, and a lack of patience with the kind of work/ing conditions typically offered; I'm mostly unemployed these days. The good news is that it frees a lot of time for thinking, coding and communicating. Any assistance with solving the rest of that equation is most appreciated.

https://www.paypal.me/codr4life

peace, out<br/>
/Andreas