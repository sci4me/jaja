#ifndef BITSET_H
#define BITSET_H

#include "array.h"

struct Bitset {
    Array<u32> data;

    void set(u64 n);
    void clear(u64 n);
    bool get(u64 n);
    void toggle(u64 n);
    u64 next_set(u64 start = 0);
    u64 next_clear(u64 start = 0);
    u64 bits_set();
};

#endif