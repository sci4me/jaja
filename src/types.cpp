#include <stdlib.h>

#include "types.h"

Allocator cstdlib_allocator = {
	.alloc = malloc,
	.realloc = realloc,
	.free = free
};