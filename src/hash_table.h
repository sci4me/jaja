#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <assert.h>
#include <stdio.h> // TODO: remove this

#include "types.h"
#include "hash.h"
#include "allocator.h"

#define HT_HASH_EMPTY 0
#define HT_HASH_REMOVED 1
#define HT_FIRST_VALID_HASH 2

#define HT_HASH_IS_OCCUPIED(x) (x >= HT_FIRST_VALID_HASH)

template <typename K>
bool default_eq_fn(K a, K b) {
    return a == b;
}

template <typename K, typename V>
struct Hash_Table {
    Allocator allocator;

    u64 (*hash_fn)(K key);
    bool (*eq_fn)(K a, K b);

    u32 size; // TODO assert size is a power of 2 always
    u32 count;
    K *keys;
    V *values;
    u64 *hashes;

private:
    void alloc() {
        assert(!keys);
        assert(!values);
        assert(!hashes);

        keys = (K*) ALLOC(allocator, size * sizeof(K));
        values = (V*) ALLOC(allocator, size * sizeof(V));
        hashes = (u64*) ALLOC(allocator, size * sizeof(u64));
    
        memset(keys, 0, size * sizeof(K)); 
        memset(values, 0, size * sizeof(V)); 
        memset(hashes, 0, size * sizeof(u64)); 
    }

    void ensure_capacity() {
        if(keys == 0) {
            alloc();
        } else if(count >= size - 2) {
            // TODO: size - 2? we ought to use load factor here?
            u32 old_size = size;
            size *= 2;

            auto old_keys = keys;
            auto old_values = values;
            auto old_hashes = hashes;

            keys = (K*) ALLOC(allocator, size * sizeof(K));
            values = (V*) ALLOC(allocator, size * sizeof(V));
            hashes = (u64*) ALLOC(allocator, size * sizeof(u64));

            memset(keys, 0, size * sizeof(K)); 
            memset(values, 0, size * sizeof(V)); 
            memset(hashes, 0, size * sizeof(u64)); 

            count = 0;
            for(u32 i = 0; i < old_size; i++) {
                if(old_hashes[i] > 1) put(old_keys[i], old_values[i]);
            }

            FREE(allocator, old_keys);
            FREE(allocator, old_values);
            FREE(allocator, old_hashes);
        }
    }

    u64 get_hash(K key) {
        u64 hash = (*hash_fn)(key);
        if(hash < HT_FIRST_VALID_HASH) hash += 2;
        return hash;
    }

public:
    Hash_Table(u64 (*_hash_fn)(K data), bool (*_eq_fn)(K a, K b) = default_eq_fn, u32 _size = 16, Allocator _allocator = cstdlib_allocator) : 
        allocator(_allocator), hash_fn(_hash_fn), eq_fn(_eq_fn), size(_size), count(0), keys(0), values(0), hashes(0) {
    }

    ~Hash_Table() {
        free();
    }

    void free() {
        if(keys) {
            assert(keys);
            assert(values);
            assert(hashes);

            FREE(allocator, keys);
            FREE(allocator, values);
            FREE(allocator, hashes);
        }
    }

    void put_by_ptr(K *key, V *value) {
        put(*key, *value);
    }

    void put(K key, V value) {
        ensure_capacity();

        u64 hash = get_hash(key);
        u64 index = hash % size;

        for(;;) {
            index &= (size - 1);
            if(hashes[index] == hash) {
                if(eq_fn(keys[index], key)) {
                    values[index] = value;
                    break;
                }
            } else if(!HT_HASH_IS_OCCUPIED(hashes[index])) {
                keys[index] = key;
                values[index] = value;
                hashes[index] = hash;
                count++;
                break;
            }
            index++;
        }
    }

    bool put_if_contains(K key, V *value) {
        if(!keys) return false;
        if(count == 0) return false;

        ensure_capacity();

        u64 hash = get_hash(key);
        u64 index = hash % size;

        for(;;) {
            index &= (size - 1);
            if(hashes[index] == hash) {
                if(eq_fn(keys[index], key)) {
                    values[index] = *value;
                    return true;
                }
            } else if(!HT_HASH_IS_OCCUPIED(hashes[index])) {
                return false;
            }
            index++;
        }
    }

    V get(K key) {
        // This is stupid but C++ sucks; apparently goto can't cross initialization of variables. OH-KAY, C++. - sci4me, Aug 19, 2019
        u64 hash = 0;
        u64 index = 0;

        if(!keys) goto _return_zero;
        if(count == 0) goto _return_zero;

        hash = get_hash(key);
        index = hash % size;

        for(;;) {
            index &= (size - 1);
            if(hashes[index] == hash) {
                if(eq_fn(keys[index], key)) {
                    return values[index];
                }
            } else if(hashes[index] == HT_HASH_EMPTY) {
                goto _return_zero;
            }
            index++;
        }

_return_zero:
        V x;
        memset(&x, 0, sizeof(V));
        return x;
    }

    V* get_ptr(K key) {
        if(!keys) return 0;
        if(count == 0) return 0;

        u64 hash = get_hash(key);
        u64 index = hash % size;

        for(;;) {
            // if(index >= size) index = 0;
            index &= (size - 1);
            if(hashes[index] == hash) {
                if(eq_fn(keys[index], key)) {
                    return &values[index];
                }
            } else if(hashes[index] == HT_HASH_EMPTY) {
                return NULL;
            }
            index++;
        }
    }

    bool contains_key(K key) {
        if(!keys) return false;
        if(count == 0) return false;

        u64 hash = get_hash(key);
        u64 index = hash % size;

        for(;;) {
            index &= (size - 1);
            if(hashes[index] == hash) {
                if(eq_fn(keys[index], key)) {
                    return true;
                }
            } else if(hashes[index] == HT_HASH_EMPTY) {
                return false;
            }
            index++;
        }

        return false;
    }

    bool remove(K key) {
        if(!keys) return false;
        if(count == 0) return false;

        u64 hash = get_hash(key);
        u64 index = hash % size;

        for(;;) {
            index &= (size - 1);
            if(hashes[index] == hash) {
                if(eq_fn(keys[index], key)) {
                    hashes[index] = HT_HASH_REMOVED;
                    count--;
                    return true;
                }
            } else if(hashes[index] == HT_HASH_EMPTY) {
                return false;
            }
            index++;
        }
    }
};

#endif