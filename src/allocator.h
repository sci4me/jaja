#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#define MEMORY_OP_ALLOC 0
#define MEMORY_OP_REALLOC 1
#define MEMORY_OP_FREE 2

typedef void* (*alloc_fn)(void *data,long unsigned int n);
typedef void* (*realloc_fn)(void *data, void* x, long unsigned int n);
typedef void (*free_fn)(void *data, void* x);

struct Allocator {
	void *data;
	alloc_fn alloc;
	realloc_fn realloc;
	free_fn free;
};

extern Allocator cstdlib_allocator;

#endif