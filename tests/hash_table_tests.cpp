#include "tests.h"
#include "../src/hash.h"
#include "../src/hash_table.h"

DEFINE_TEST(put_works) {
    auto x = Hash_Table<u64, u64>(hash_u64);

    x.put(1, 42);

    assert(x.count == 1);

    u32 i = 0;
    while(x.state[i] != HT_STATE_OCCUPIED) i++;

    assert(x.keys[i] == 1);
    assert(x.values[i] == 42);
}

DEFINE_TEST(get_works) {
    auto x = Hash_Table<u64, u64>(hash_u64);

    x.put(1, 42);
    x.put(2, 69);
    x.put(1337, 999);

    assert(x.get(1) == 42);
    assert(x.get(2) == 69);
    assert(x.get(1337) == 999);
}

DEFINE_TEST(contains_works) {
    auto x = Hash_Table<u64, u64>(hash_u64);

    x.put(3, 42);
    x.put(7, 69);
    x.put(11, 8675309);
    x.put(6789998212, 1);

    assert(x.contains_key(3));
    assert(x.contains_key(7));
    assert(x.contains_key(11));
    assert(x.contains_key(6789998212));

    assert(!x.contains_key(22));
}

DEFINE_TEST(remove_works) {
    auto x = Hash_Table<u64, u64>(hash_u64);

    x.put(3, 42);
    x.put(7, 69);
    x.put(11, 8675309);
    x.put(6789998212, 1);

    assert(x.remove(7));
    assert(x.remove(6789998212));

    assert(x.contains_key(3));
    assert(!x.contains_key(7));
    assert(x.contains_key(11));
    assert(!x.contains_key(6789998212));
}