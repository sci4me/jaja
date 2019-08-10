#include "test.h"
#include "../src/bitset.h"

DEFINE_TEST(bitset_set_works) {
    auto x = Bitset();

    x.set(4);
    x.set(5);

    assert(x.data.count == 1);
    assert(x.data.data[0] == (1 << 4) | (1 << 5));

    x.set(32);

    assert(x.data.count == 2);

    assert(x.data.data[0] == (1 << 4) | (1 << 5));
    assert(x.data.data[1] == 1);
}

DEFINE_TEST(bitset_clear_works) {
    auto x = Bitset();

    x.set(4);
    x.set(37);

    assert(x.data.data[0] == (1 << 4));
    assert(x.data.data[1] == (1 << (37 % 32)));

    x.clear(4);
    x.clear(37);

    assert(x.data.data[0] == 0);
    assert(x.data.data[1] == 0);
}

DEFINE_TEST(bitset_toggle_works) {
    auto x = Bitset();

    x.toggle(3);
    x.toggle(42);

    assert(x.data.count == 2);

    assert(x.data.data[0] == (1 << 3));
    assert(x.data.data[1] == (1 << (42 % 32)));

    x.toggle(3);
    x.toggle(42);

    assert(x.data.data[0] == 0);
    assert(x.data.data[1] == 0);
}

DEFINE_TEST(bitset_get_works) {
    auto x = Bitset();

    x.set(3);
    x.set(7);
    x.set(12);
    x.set(70);

    assert(x.data.count == 3);

    for(u32 i = 0; i < 80; i++) {
        auto expected = i == 3 || i == 7 || i == 12 || i == 70;
        assert(x.get(i) == expected);
    }
}

DEFINE_TEST(bitset_next_set_works) {
    auto x = Bitset();

    x.set(7);
    x.set(26);
    x.set(45);

    assert(x.next_set() == 7);
    assert(x.next_set(3) == 7);
    assert(x.next_set(8) == 26);
    assert(x.next_set(26) == 26);
    assert(x.next_set(27) == 45);
}

DEFINE_TEST(bitset_next_clear_works) {
    auto x = Bitset();

    x.set(0);
    x.set(7);
    x.set(22);

    assert(x.next_clear() == 1);
    assert(x.next_clear(3) == 3);
    assert(x.next_clear(7) == 8);
    assert(x.next_clear(22) == 23);
}