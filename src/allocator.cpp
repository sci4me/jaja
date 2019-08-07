#include <stdlib.h>

#include "allocator.h"
#include "types.h"

static void* __malloc(void *data, unsigned long int n) {
	return malloc(n);
}

static void __free(void *data, void *x) {
	free(x);
}

Allocator cstdlib_allocator = {
	.data = NULL,
	.alloc = __malloc,
	.free = __free
};