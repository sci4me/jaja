#include <assert.h>

#include "test.h"
#include "../src/arena.h"

DEFINE_TEST(alloc_works) {
    auto a = Arena();

    auto x = a.alloc(42);

    assert(x == a.head->data);
}