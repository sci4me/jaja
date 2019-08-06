#include "runtime.h"

#ifdef HEAP_DEBUG
Value* Heap::alloc(u32 line, const char *func, const char *file) {
#else
Value* Heap::alloc() {
#endif
	if(allocations > 1000) {
		gc();
	}

	auto a = new Allocation();
	// printf("alloc a: %p : %s@%s:%d\n", a, func, file, line);
	a->next = head;
	a->value.a = a;
	head = a;
	allocations++;
#ifdef HEAP_DEBUG
	a->line = line;
	a->func = func;
	a->file = file;
#endif
	return &a->value;
}

void Heap::mark_root(Value v) {
	if(!v.a) return;
	assert(v.a->next != v.a);

	roots.add(v);
}

void Heap::unmark_root(Value v) {
	if(!v.a) return;
	assert(v.a->next != v.a);

	auto i = roots.index_of(v);
	if(i != -1)	roots.unordered_remove(i);
}

void Heap::gc() {
	u32 total = allocations;
	u32 marked = 0;

	FOR((&roots), i) {
		auto root = roots.data[i];
		marked += mark(root);
	}
	u32 swept = sweep();
	allocations -= swept;

	printf("\nGC Cycle:\n\tstart:  %u\n\tmarked: %u\n\tswept:  %u\n\tmissing: %u\n\tend: %u\n", total, marked, swept, total - (marked + swept), allocations);
}

u32 Heap::mark(Value v) {
	if(!v.a) return 0;
	assert(v.a->next != v.a);

	v.a->marked = true;

	u32 marked = 1;

	if(v.type == VALUE_OBJECT) {
		for(u32 i = 0; i < v.object->count; i++) {
			marked += mark(v.object->keys[i]);
			marked += mark(v.object->values[i]);
		} 
	}

	return marked;
}

u32 Heap::sweep() {
	Array<Allocation*> to_delete; // TODO HACK REMOVEME
	Allocation *prev = NULL;
	for(Allocation *curr = head; curr; prev = curr, curr = curr->next) {
		assert(curr->next != curr);

		if(curr->marked) continue;

		auto next = curr->next;
		if(prev) {
			assert(prev != next);
			prev->next = next;
		} else {
			head = next;
		}

		// printf("sweeping %d (%d) : %s@%s:%d\n", curr->value.type, curr->marked, curr->func, curr->file, curr->line);

		to_delete.add(curr);
	}	
	FOR((&to_delete), i) {	
		// TODO: this is wrong
		delete to_delete.data[i];
	}

	return to_delete.count;
}