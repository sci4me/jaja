#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdlib.h>
#include <assert.h>

#include "types.h"
#include "hash.h"

#include <stdio.h>

#define HT_STATE_EMPTY 0
#define HT_STATE_OCCUPIED 1
#define HT_STATE_REMOVED 2

template <typename K, typename V>
struct Hash_Table {
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

            keys = (K*) calloc(size, sizeof(K));
            values = (V*) calloc(size, sizeof(V));
            state = (u8*) calloc(size, sizeof(u8));

            for(u32 i = 0; i < old_size; i++) {
                if(old_state[i]) put(old_keys[i], old_values[i]);
            }

            free(old_keys);
            free(old_values);
            free(old_state);
        }
    }

    Hash_Table(u64 (*_hash_fn)(K data), bool (*_eq_fn)(K a, K b), u32 _size = 16) : hash_fn(_hash_fn), eq_fn(_eq_fn), size(_size) {
        assert(size > 0);
        count = 0;
        keys = (K*) calloc(size, sizeof(K));
        values = (V*) calloc(size, sizeof(V));
        state = (u8*) calloc(size, sizeof(u8));
    }

    ~Hash_Table() {
        free(keys);
        free(values);
        free(state);
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
        memset(&x, 0, sizeof(x));
        return x;
    }

    bool remove(K key) {
        u64 index = hash_fn(key) % size;
        for(;;) {
            index &= (size - 1);
            if(state[index] == HT_STATE_OCCUPIED && eq_fn(keys[index], key)) {
                memset(&keys[index], 0, sizeof(K));
                memset(&values[index], 0, sizeof(V));
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