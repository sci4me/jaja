#include "runtime.h"

#ifdef HEAP_DEBUG
Value* Heap::alloc(u32 line, const char *func, const char *file) {
#else
Value* Heap::alloc() {
#endif
	if(allocations.count > 1000) {
		gc();
	}

#ifdef HEAP_DEBUG
	auto a = new Allocation(line, func, file);
#else
	auto a = new Allocation();
#endif
	// printf("alloc a: %p : %s@%s:%d\n", a, func, file, line);
	// a->next = head;
	a->value.a = a;
	// head = a;
	allocations.push(a);
	return &a->value;
}

void Heap::mark_root(Allocation *a) {
	assert(a);
	assert(allocations.index_of(a) != -1);

	// assert(v.a->next != v.a);

	roots.push(a);
}

void Heap::unmark_root(Allocation *a) {
	assert(a);
	assert(allocations.index_of(a) != -1);
	// assert(v.a->next != v.a);

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
	// u32 swept = 0;

	printf("\nGC Cycle:\n\tstart:  %u\n\tmarked: %u\n\tswept:  %u\n\tmissing: %u\n\tend: %u\n", total, marked, swept, total - (marked + swept), allocations.count);
}

u32 Heap::mark(Allocation *a) {
	assert(a);
	assert(allocations.index_of(a) != -1);

	// if(allocations.index_of(a) == -1) {
		// printf("skipped marking degenerate allocation: %p : %s@%s:%u\n", a, a->func, a->file, a->line);
		// return 0;
	// }
	
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
	u32 swept = 0;

	FOR((&allocations), i) {
		auto x = allocations.data[i];
		if(x->marked) {
			_allocations.push(x);
		} else {
			swept++;

			delete x;
		}
	}

	auto tmp = allocations;
	allocations = _allocations;
	_allocations = tmp;
	_allocations.clear();

	return swept;

	/*
	Array<Allocation*> to_remove;
	FOR((&allocations), i) {
		auto x = allocations.data[i];
		if(!x->marked) {
			x->marked = false;
		} else {
			to_remove.push(x);
		}
	}
	FOR((&to_remove), i) {
		auto x = allocations.data[i];
		allocations.unordered_remove(allocations.index_of(x));
		delete x;
	}
	*/

	// Array<Allocation*> to_delete; // TODO HACK REMOVEME
	// Allocation *prev = NULL;
	// for(Allocation *curr = head; curr; prev = curr, curr = curr->next) {
	// 	assert(curr->next != curr);

	// 	if(curr->marked) continue;

	// 	auto next = curr->next;
	// 	if(prev) {
	// 		assert(prev != next);
	// 		prev->next = next;
	// 	} else {
	// 		head = next;
	// 	}

	// 	// printf("sweeping %d (%d) : %s@%s:%d\n", curr->value.type, curr->marked, curr->func, curr->file, curr->line);

	// 	to_delete.push(curr);
	// }	
	// FOR((&to_delete), i) {	
	// 	// TODO: this is wrong
	// 	delete to_delete.data[i];
	// }

	// return to_delete.count;

	// return to_remove.count;
}