#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c4.h"
#include "coro.h"
#include "ctx.h"
#include "db/col.h"
#include "db/cols/str_col.h"
#include "db/rec.h"
#include "db/tbl.h"
#include "defer.h"
#include "err.h"
#include "mem/mfreel.h"
#include "mem/mpool.h"
#include "mem/mslab.h"
#include "seqs/bmap.h"
#include "seqs/dyna.h"
#include "seqs/ls.h"
#include "seqs/pair.h"
#include "val.h"
#include "utils.h"

static int int_cmp(void *_x, void *_y, void *data) {
  int *x = _x, *y = _y;
  if  (*x < *y) return -1;
  return *x > *y;
}

static void bmap_add_tests() {
  C4BMAP(m, sizeof(int), sizeof(char), int_cmp);

  for (int i = 0; i < 10; i++) {
    struct c4pair *it = c4bmap_add(&m, &i);
    *(int *)c4pair_left(it) = i;
    *(char *)c4pair_right(it) = 'a' + i;
  }
    
  for (int i = 0; i < 10; i++) {
    assert(*(char *)c4bmap_get(&m, &i) == 'a' + i);
  }
  
  c4bmap_free(&m);
}

static void bmap_seq_tests() {
  C4BMAP(m, sizeof(int), sizeof(char), int_cmp);

  for (int i = 0; i < 10; i++) {
    struct c4pair *it = c4bmap_add(&m, &i);
    *(int *)c4pair_left(it) = i;
    *(char *)c4pair_right(it) = 'a' + i;
  }

  int i = 0;

  C4SEQ(c4bmap, &m, seq);
  for (struct c4pair *it; (it = c4seq_next(seq));) {
    assert(*(int *)c4pair_left(it) == i);
    i++;
  }

  c4bmap_free(&m);
}

static void bmap_set_tests() {
  C4BMAP(m, sizeof(int), sizeof(char), int_cmp);

  for (int i = 0; i < 5; i++) {
    struct c4pair *it = c4bmap_add(&m, &i);
    *(int *)c4pair_left(it) = i;
    *(char *)c4pair_right(it) = 'a' + i;
  }
  
  for (int i = 0; i < 10; i++) {
    struct c4pair *it = c4bmap_set(&m, &i);
    if (i > 4) { *(int *)c4pair_left(it) = i; }
    *(char *)c4pair_right(it) = 'z' - i;    
  }
  
  for (int i = 0; i < 10; i++) {
    assert(*(char *)c4bmap_get(&m, &i) == 'z' - i);
  }
  
  c4bmap_free(&m);
}

static void bmap_tests() {
  bmap_add_tests();
  bmap_seq_tests();
  bmap_set_tests();
}

static void bset_tests() {
  static int MAX = 100;
  C4BSET(set, sizeof(int), int_cmp);
  for (int i = 0; i < MAX; i++) { *(int *)c4bset_add(&set, &i) = i; }

  assert(cbset_len(&set) == MAX);
  
  for (int i = 0; i < MAX; i++) {
    assert(*(int *)c4bset_get(&set, &i) == i);
    assert(*(int *)c4bset_idx(&set, i) == i);
  }

  c4bset_free(&set);
}

static void col_tests() {
  struct c4val_t type;
  c4val_t_init(&type, "foo");

  struct c4col col;
  c4col_init(&col, "bar", &type);
  
  c4col_free(&col);
  c4val_t_free(&type);
}

struct coro_ctx { int i, line; };

static int coro(struct coro_ctx *ctx, int foo) {
  C4CORO(&ctx->line);
  for(ctx->i = 1; ctx->i <= 10; ctx->i++) { C4CORO_RET(foo + ctx->i); }
  C4CORO_END();
  
  return -1;
}

static void coro_tests() {
  struct coro_ctx ctx = {0, 0};
  for (int i = 1; i <= 10; i++) { assert(coro(&ctx, i) == i*2); }
  assert(coro(&ctx, 0) == -1);
}

static void defer_tests() {
  bool called = false;

  {
    C4DEFER({ called = true; });
    assert(!called);
  }
  
  assert(called);
}

static void defer_scope_tests() {
  int called = false;
  
  C4DEFER_SCOPE(outer) {
    C4DEFER_SCOPE(inner) {
      C4DEFER_TO(outer, { called = true; });
    }
    
    assert(!called);
  }

  assert(called);
}

static void dyna_tests() {
  // Declare and initialize dynamic array named arr for int sized items
  
  C4DYNA(arr, sizeof(int));
  const int LEN = 10;

  // Preallocate to fit at least LEN/2 items

  c4dyna_grow(&arr, LEN/2);

  // Populate array

  for (int i = 0; i < LEN; i++) { *(int *)c4dyna_push(&arr) = 1; }
  assert(arr.len == LEN);

  // Empty array and check reverse order

  for (int i = LEN-1; i >= 0; i--) { *(int *)c4dyna_pop(&arr) = i; }
  assert(arr.len == 0);

  c4dyna_free(&arr);
}

static struct c4err_t custom_type;

static void err_tests() {
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

static void lambda_tests() {
  assert(C4LAMBDA({ return x*y; }, int, int x, int y)(2, 3) == 6);
}

static void ls_splice_tests() {
  struct c4ls foo;
  struct c4ls bar;
  struct c4ls its[10];

  c4ls_init(&foo);
  c4ls_init(&bar);
  
  for (int i = 0; i < 5; i++) {
    c4ls_prepend(&foo, its+i);
    c4ls_prepend(&bar, its+i+5);    
  }

  c4ls_splice(&foo, bar.next, bar.prev);
  struct c4ls *it = &foo;
  
  for (int i = 0; i < 10; i++) {
    it = it->next;
    assert(it == its+i);
  }

  assert(it->next == &foo);
}

static void ls_tests() {
  ls_splice_tests();
}

static void mfreel_tests() {
  // Define and initialize with default source
  
  C4MPOOL(mp, &c4malloc);
  C4MFREEL(mf, &mp);
  C4DEFER({ c4mfreel_free(&mf); c4mpool_free(&mp); });

  const int LEN = 10;
  void *ptrs[LEN];

  for (int i = 0; i < LEN; i++) {
    ptrs[i] = c4mfreel_acquire(&mf, sizeof(int));
  }

  // Release all memory to freelist
  
  for (int i = 0; i < LEN; i++) { c4mfreel_release(&mf, ptrs[i]); }

  for (int i = LEN-1; i >= 0; i--) {
    // Make sure that memory is recycled by freelist
    
    assert(c4mfreel_acquire(&mf, sizeof(int)) == ptrs[i]);
  }
}

static void mpool_tests() {
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

static void mslab_tests() {
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

static void pair_tests() {
  struct c4pair *p = c4pair(sizeof(int), sizeof(bool));
  *(int *)c4pair_left(p) = 42;
  *(bool *)c4pair_right(p) = false;
  c4release(p);
}

static void rec_tests() {
  struct c4str_col foo;
  c4str_col_init(&foo, "foo");

  struct c4rec rec;
  c4rec_init(&rec, NULL);
  c4set_str(&rec, &foo, "abc");
  assert(strcmp(c4get_str(&rec, &foo), "abc") == 0);

  c4rec_free(&rec);
  c4str_col_free(&foo);
}

static void tbl_seq_tests() {
  struct c4tbl tbl;
  c4tbl_init(&tbl, "foo");
 
  struct c4rec rec;
  c4rec_init(&rec, NULL);
  c4tbl_upsert(&tbl, &rec);

  C4SEQ(c4tbl, &tbl, seq);  
  assert(c4uid_cmp(((struct c4rec *)c4seq_next(seq))->id, rec.id) == 0);
  assert(c4seq_next(seq) == NULL);

  c4rec_free(&rec);
  c4tbl_free(&tbl);
}

static void tbl_tests() {
  tbl_seq_tests();
}

void malloc_perf_tests();

int main() {
  c4init();

  C4TRY("run all tests") {
    bmap_tests();
    bset_tests();
    col_tests();
    coro_tests();
    defer_tests();
    defer_scope_tests();
    dyna_tests();
    err_tests();
    lambda_tests();
    ls_tests();
    malloc_perf_tests();
    mfreel_tests();
    mpool_tests();
    mslab_tests();
    pair_tests();
    rec_tests();
    tbl_tests();

    //C4THROW(&c4err, "test print");
  }

  c4free();
  c4ctx_free(c4ctx());
  return 0;
}
