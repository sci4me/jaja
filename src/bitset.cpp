#include "bitset.h"

#define WORD_SIZE 32

static void ensure(Array<u32> *data, u64 n) {
    while(data->count <= n) data->push(0);
}

void Bitset::set(u64 n) {
    auto e = n / WORD_SIZE;
    auto i = n % WORD_SIZE;
    ensure(&data, e);
    data.data[e] |= (1 << i);
}

void Bitset::clear(u64 n) {
    auto e = n / WORD_SIZE;
    auto i = n % WORD_SIZE;
    ensure(&data, e);
    data.data[e] &= ~(1 << i);
}

bool Bitset::get(u64 n) {
    auto e = n / WORD_SIZE;
    auto i = n % WORD_SIZE;
    ensure(&data, e);
    return data.data[e] & (1 << i);
}

void Bitset::toggle(u64 n) {
    auto e = n / WORD_SIZE;
    auto i = n % WORD_SIZE;
    ensure(&data, e);
    data.data[e] ^= (1 << i);
}

u64 Bitset::next_set(u64 start) {
    u64 n = start;
    while(!get(n)) n++;
    return n;
}

u64 Bitset::next_clear(u64 start) {
    u64 n = start;
    while(get(n)) n++;
    return n;
}