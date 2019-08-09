#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <assert.h>

#include "types.h"
#include "hash.h"
#include "allocator.h"

#include <stdio.h>

#define HT_STATE_EMPTY 0
#define HT_STATE_OCCUPIED 1
#define HT_STATE_REMOVED 2

template <typename K>
bool default_eq_fn(K a, K b) {
    return a == b;
}

template <typename K, typename V>
struct Hash_Table {
    Allocator allocator;

    u64 (*hash_fn)(K key);
    bool (*eq_fn)(K a, K b);

    u32 size;
    u32 count;
    K *keys;
    V *values;
    u8 *state;

    void ensure_capacity() {
        if(count >= size - 2) {
            u32 old_size = size;
            size *= 2;

            auto old_keys = keys;
            auto old_values = values;
            auto old_state = state;

            keys = (K*) ALLOC(allocator, size * sizeof(K));
            values = (V*) ALLOC(allocator, size * sizeof(V));
            state = (u8*) ALLOC(allocator, size * sizeof(u8));

            for(u32 i = 0; i < old_size; i++) {
                if(old_state[i]) put(old_keys[i], old_values[i]);
            }

            FREE(allocator, old_keys);
            FREE(allocator, old_values);
            FREE(allocator, old_state);
        }
    }

    Hash_Table(u64 (*_hash_fn)(K data), bool (*_eq_fn)(K a, K b) = default_eq_fn, u32 _size = 16, Allocator _allocator = cstdlib_allocator) : allocator(_allocator), hash_fn(_hash_fn), eq_fn(_eq_fn), size(_size) {
        assert(size > 0);
        count = 0;
        keys = (K*) ALLOC(allocator, size * sizeof(K));
        values = (V*) ALLOC(allocator, size * sizeof(V));
        state = (u8*) ALLOC(allocator, size * sizeof(u8));
    }

    ~Hash_Table() {
        FREE(allocator, keys);
        FREE(allocator, values);
        FREE(allocator, state);
    }

    void put(K key, V value) {
        ensure_capacity();

        u64 index = hash_fn(key) % size;
        for(;;) {
            index &= (size - 1);
            if(state[index] == HT_STATE_OCCUPIED) {
                if(eq_fn(keys[index], key)) {
                    values[index] = value;
                    break;
                }
            } else{
                keys[index] = key;
                values[index] = value;
                state[index] = HT_STATE_OCCUPIED;
                count++;
                break;
            }
            index++;
        }
    }

    V get(K key) {
        u64 index = hash_fn(key) % size;
        for(;;) {
            index &= (size - 1);
            if(state[index] == HT_STATE_OCCUPIED && eq_fn(keys[index], key)) {
                return values[index];
            } else if(state[index] == HT_STATE_EMPTY) {
                goto _return_zero;
            }
            index++;
        }

_return_zero:
        V x;
        memset(&x, 0, sizeof(V));
        return x;
    }

    bool contains_key(K key) {
        u64 index = hash_fn(key) % size;
        for(;;) {
            index &= (size - 1);
            if(state[index] == HT_STATE_OCCUPIED && eq_fn(keys[index], key)) {
                return true;
            } else if(state[index] == HT_STATE_EMPTY) {
                return false;
            }
            index++;
        }

        return false;
    }

    bool remove(K key) {
        u64 index = hash_fn(key) % size;
        for(;;) {
            index &= (size - 1);
            if(state[index] == HT_STATE_OCCUPIED && eq_fn(keys[index], key)) {
                // memset(&keys[index], 0, sizeof(K));
                // memset(&values[index], 0, sizeof(V));
                state[index] = HT_STATE_REMOVED;
                count--;
                return true;
            } else if(state[index] == HT_STATE_EMPTY) {
                return false;
            }
            index++;
        }
    }
};

#endif