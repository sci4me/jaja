#include <alloca.h>
#include <stdio.h>

#include "test.h"
#include "../src/hash.h"
#include "../src/hash_table.h"

DEFINE_TEST(hash_table_put_works) {
    auto x = Hash_Table<u64, u64>(hash_u64);

    x.put(1, 42);

    assert(x.count == 1);

    u32 i = 0;
    while(x.hashes[i] < 2) i++;

    assert(x.keys[i] == 1);
    assert(x.values[i] == 42);
}

DEFINE_TEST(hash_table_put_if_contains_works) {
    auto x = Hash_Table<u64, u64>(hash_u64);

    x.put(5, 2);

    u64 v = 7;

    assert(x.put_if_contains(5, &v));
    assert(!x.put_if_contains(6, &v));

    assert(x.get(5) == 7);
    assert(!x.contains_key(6));   
}

DEFINE_TEST(hash_table_get_works) {
    auto x = Hash_Table<u64, u64>(hash_u64);

    x.put(1, 42);
    x.put(2, 69);
    x.put(1337, 999);

    assert(x.get(1) == 42);
    assert(x.get(2) == 69);
    assert(x.get(1337) == 999);

    assert(x.get(11) == 0);
}

DEFINE_TEST(hash_table_contains_works) {
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

DEFINE_TEST(hash_table_remove_works) {
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

DEFINE_TEST(hash_table_stress_test) {
    auto x = Hash_Table<u64, u64>(hash_u64);

    const u32 n = 1000;

    for(u32 i = 0; i < n; i++) x.put(i * 2, i * 3);

    assert(x.count == n);

    for(u32 i = 0; i < n; i++) {
        assert(x.contains_key(i * 2));
        assert(x.get(i * 2) == i * 3);
    }
}

DEFINE_TEST(hash_table_string_stress_test) {
    auto x = Hash_Table<char*, u32>(hash_string, eq_string);

    const u32 N = 1000;

    for(u32 i = 0; i < N; i++) {
        char *buf = (char*) alloca(4 + 3 + 1);
        sprintf(buf, "test%u", i);
        x.put(buf, i + 1);
    }

    for(u32 i = 0; i < N; i++) {
        char *buf = (char*) alloca(4 + 3 + 1);
        sprintf(buf, "test%u", i);
        assert(x.get(buf) == i + 1);
    }
}