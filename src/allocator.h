#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#define MEMORY_OP_ALLOC 0
#define MEMORY_OP_REALLOC 1
#define MEMORY_OP_FREE 2

#define ALLOCATOR_DEBUG

#ifdef ALLOCATOR_DEBUG
	typedef void* (*alloc_fn)(void *data,long unsigned int n, unsigned int line, const char *func, const char *file);
#else
	typedef void* (*alloc_fn)(void *data,long unsigned int n);
#endif

typedef void (*free_fn)(void *data, void* x);

struct Allocator {
	void *data;
	alloc_fn alloc;
	free_fn free;
};

extern Allocator cstdlib_allocator;

#ifdef ALLOCATOR_DEBUG
	#define ALLOC(allocator, n) allocator.alloc(allocator.data, n, __LINE__, __func__, __FILE__)
#else
	#define ALLOC(allocator, n) allocator.alloc(allocator.data, n)
#endif

#define FREE(allocator, x) allocator.free(allocator.data, x)

#endif