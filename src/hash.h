#ifndef HASH_H
#define HASH_H

#include <string.h>

#include "types.h"

inline u64 hash_u64(u64 x) {
    x *= 0xff51afd7ed558ccd;
    x ^= x >> 32;
    return x;
}

inline u64 hash_mix(u64 a, u64 b) {
    a ^= b;
    a *= 0xff51afd7ed558ccd;
    a ^= a >> 32;
    return a;
}

inline u64 hash_ptr(void *ptr) {
    return hash_u64((u64) ptr);
}

inline u64 hash_string(char *str) {
    u64 x = 0xcbf29ce484222325;
    for(u64 i = 0; i < strlen(str); i++) {
        x ^= str[i];
        x *= 0x100000001b3;
        x ^= x >> 32;
    }
    return x;
}

inline bool eq_string(char *a, char *b) {
    return strcmp(a, b) == 0;
}

#endif