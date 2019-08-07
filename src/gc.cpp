#include <stdlib.h>

#include "runtime.h"

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

static void free_allocation(Allocation *a) {
	switch(a->value.type) {
		case VALUE_STRING:
		case VALUE_REFERENCE:
			break;
		case VALUE_OBJECT:
			delete a->value.object;
			break;
		case VALUE_LAMBDA:
			// TODO why is this no bueno?
			// jit_free(a->value.lambda.j);
			break;
	}

	free(a);
}

Heap::~Heap() {
	FOR((&allocations), i) {
		free_allocation(allocations.pop());	
	}
}

#ifdef HEAP_DEBUG
Value* Heap::alloc(u32 line, const char *func, const char *file) {
#else
Value* Heap::alloc() {
#endif
	if(allocations.count > 1000) {
		gc();
	}

	auto a = (Allocation*) malloc(sizeof(Allocation));
#ifdef HEAP_DEBUG
	a->line = line;
	a->func = func;
	a->file = file;
#endif
	// printf("alloc a: %p : %s@%s:%d\n", a, func, file, line);
	a->marked = false;
	a->value.a = a;
	allocations.push(a);
	return &a->value;
}

void Heap::mark_root(Allocation *a) {
	assert(a);
	assert(allocations.index_of(a) != -1);

	// TODO should we really allow this if statement?
	if(roots.index_of(a) == -1) roots.push(a);
}

void Heap::unmark_root(Allocation *a) {
	assert(a);
	assert(allocations.index_of(a) != -1);

	auto i = roots.index_of(a);
	if(i != -1)	roots.unordered_remove(i);
}

void Heap::gc() {
	u32 total = allocations.count; 

	u32 marked = 0;
	FOR((&roots), i) {
		auto root = roots.data[i];
		assert(allocations.index_of(root) != -1);
		marked += mark(root);
	}

	u32 k = 0;
	FOR((&allocations), i) {
		if(allocations.data[i]->marked) k++;
	}
	assert(k == marked);

	u32 swept = sweep();

	// printf("\nGC Cycle:\n\tstart:  %u\n\tmarked: %u\n\tswept:  %u\n\tmissing: %u\n\tend: %u\n", total, marked, swept, total - (marked + swept), allocations.count);
}

u32 Heap::mark(Allocation *a) {
	assert(a);
	assert(allocations.index_of(a) != -1);

	// assert(v.a->next != v.a);

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
	Array<Allocation*> to_remove;
	FOR((&allocations), i) {
		auto x = allocations.data[i];
		if(x->marked) {
			x->marked = false;
		} else {
			to_remove.push(x);
		}
	}
	FOR((&to_remove), i) {
		auto x = to_remove.data[i];
		allocations.unordered_remove(allocations.index_of(x));
		free_allocation(x);
	}

	return to_remove.count;
}