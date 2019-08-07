#ifndef TYPES_H
#define TYPES_H

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;
typedef unsigned long long u64;
typedef long long s64;
typedef float f32;
typedef double f64;

#define MEMORY_OP_ALLOC 0
#define MEMORY_OP_REALLOC 1
#define MEMORY_OP_FREE 2

typedef void* (*alloc_fn)(long unsigned int n);
typedef void* (*realloc_fn)(void* x, long unsigned int n);
typedef void (*free_fn)(void* x);

struct Allocator {
	alloc_fn alloc;
	realloc_fn realloc;
	free_fn free;
};

extern Allocator cstdlib_allocator;

#endif