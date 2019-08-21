#include <stdio.h>

#include "runtime.h"
#include "allocator.h"

// TODO make GC/Heap _MUCH_ better
//  - objects want the ability to have multiple allocations that get marked when that object gets marked
//    - things like hash tables and arrays that resize
//    - ability to free for extra credit?
//      - free list type of thing?
//  - do we want plain-old mark/sweep? (not really)
//    - generational
//    - moving vs. non-moving
//    - incremental? (HUGE bonus points)
//  - proper tracking of memory usage and limits
//  - resizing (i.e. get bigger when needed and smaller when it makes sense*)

static void free_allocation(Allocator allocator, Allocation *a) {
	if(a->value.type == VALUE_OBJECT) {
		a->value.object->free();
		FREE(allocator, a->value.object);
	}

	FREE(allocator, a);
}

#ifdef HEAP_DEBUG
Value* Heap::alloc(u32 line, const char *func, const char *file) {
#else
Value* Heap::alloc() {
#endif
	// TODO: percentage? idk fam
	if(allocations > 1000) {
		gc();
	}

	auto a = (Allocation*) ALLOC(allocator, sizeof(Allocation));
#ifdef HEAP_DEBUG
	a->line = line;
	a->func = func;
	a->file = file;
	printf("alloc a: %p : %s@%s:%d\n", a, func, file, line);
#endif
	a->marked = false;
	a->value.a = a;
	
	a->next = head;
	head = a;
	allocations++;

	return &a->value;
}

void Heap::mark_root(Allocation *a) {
	assert(a);
	// TODO: assert a is in our set of allocations

	roots.push(a);
}

void Heap::unmark_root(Allocation *a) {
	assert(a);
	// TODO: assert a is in our set of allocations

	auto i = roots.index_of(a);
	if(i != -1)	roots.unordered_remove(i); // TODO: should we really have this if statement? or should we assert i != -1?
}

void Heap::gc() {
	u32 total = allocations; 

	u32 marked = 0;
	FOR((&roots), i) {
		auto root = roots.data[i];
		// assert(allocations.index_of(root) != -1);
		marked += mark(root);
	}

	u32 swept = sweep();
	allocations -= swept;

	printf("\nGC Cycle:\n\tstart:   %u\n\tmarked:  %u\n\tswept:   %u\n\tmissing: %u\n\tend:     %u\n", total, marked, swept, total - (marked + swept), allocations);
}

u32 Heap::mark(Allocation *a) {
	assert(a);
	// assert(allocations.index_of(a) != -1);

	if(a->marked) return 0;

	a->marked = true;

	u32 marked = 1;

	if(a->value.type == VALUE_OBJECT) {
		auto obj = a->value.object;
		for(u32 i = 0; i < obj->count; i++) {
			auto key = obj->keys[i];
			auto value = obj->values[i];
			if(key.a) marked += mark(key.a);
			if(value.a) marked += mark(value.a);
		} 
	}

	return marked;
}

u32 Heap::sweep() {
	u32 swept = 0;

	Allocation *prev = NULL;
	Allocation *curr = head;

	while(curr) {
		auto next = curr->next;

		if(curr->marked) {
			curr->marked = false;
		} else {
			if(prev) {
				prev->next = next;
			} else {
				head = next;
			}

			free_allocation(allocator, curr);
			swept++;

			curr = next;
		}

		prev = curr;
		curr = next;
	}

	return swept;
}