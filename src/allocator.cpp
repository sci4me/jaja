#include <stdlib.h>
#include <assert.h>

#include "allocator.h"
#include "types.h"

#ifdef ALLOCATOR_DEBUG
struct Allocation {
	Allocation *next;

	void *ptr;
	u32 line;
	const char *func;
	const char *file;
};

static Allocation *head;

static void* __malloc(void *data, unsigned long int n, u32 line, const char *func, const char *file) {
	auto ptr =  malloc(n);

	auto next = (Allocation*) malloc(sizeof(Allocation));
	next->next = head;
	next->ptr = ptr;
	next->func = func;
	next->file = file;
	head = next;

	return ptr;
}

static void __free(void *data, void *x) {
	assert(x);

	Allocation *prev = NULL;
	Allocation *curr = head;

	while(curr) {
		auto next = curr->next;

		if(curr->ptr == x) {
			if(prev) {
				prev->next = next;
			} else {
				head = next;
			}

			free(curr);
			free(x);
			return;
		}

		prev = curr;
		curr = next;	
	}

	assert(false);
}	
#else
static void* __malloc(void *data, unsigned long int n) {
	return malloc(n);
}

static void __free(void *data, void *x) {
	free(x);
}
#endif

Allocator cstdlib_allocator = {
	.data = NULL,
	.alloc = __malloc,
	.free = __free
};