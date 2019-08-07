#include <stdlib.h>

#include "allocator.h"
#include "types.h"

static void* __malloc(void *data, unsigned long int n) {
	return malloc(n);
}

static void* __realloc(void *data, void *x, unsigned long int n) {
	return realloc(x, n);
}

static void __free(void *data, void *x) {
	free(x);
}

Allocator cstdlib_allocator = {
	.data = NULL,
	.alloc = __malloc,
	.realloc = __realloc,
	.free = __free
};