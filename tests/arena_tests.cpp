#include <assert.h>

#include "test.h"
#include "../src/arena.h"

DEFINE_TEST(arena_alloc_works) {
    auto a = Arena();

    auto x = a.alloc(42);

    assert(x == a.head->data);
    assert(a.head->used == 42);
}

DEFINE_TEST(arena_expands_as_needed) {
    auto a = Arena(cstdlib_allocator, 10);

    assert(a.block_count == 1);

    auto x = a.alloc(10);

    assert(a.block_count == 1);
    assert(!a.head->next);
    assert(a.head->used == 10);
    assert(x == a.head->data);

    auto y = a.alloc(1);

    assert(a.block_count == 2);
    assert(a.head->next);
    assert(a.head->used == 1);
    assert(y == a.head->data);
}