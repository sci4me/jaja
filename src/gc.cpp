#include "gc.h"

Value* Heap::alloc() {
	auto v = new Value();
	auto a = new Allocation();
	a->next = head;
	a->value = v;
	head = a;
	return v;
}

void Heap::mark_root(Value *v) {
	roots.add(v);
}

void Heap::unmark_root(Value *v) {
	auto i = roots.index_of(v);
	assert(i != -1);
	roots.unordered_remove(i);
}

void Heap::gc() {
	FOR((&roots), i) {
		mark(roots.data[i]);
	}

	sweep();
}

void Heap::mark(Value *v) {
	v->a->marked = true;

	if(v->type == VALUE_OBJECT) {
		for(u32 i = 0; i < v->object->count; i++) {
			mark(v->object->keys[i]);
			mark(v->object->values[i]);
		} 
	}
}

void Heap::sweep() {
	Allocation *prev = NULL;
	for(Allocation *curr = head; curr; prev = curr, curr = curr->next) {
		if(curr->marked) continue;

		if(prev) {
			prev->next = curr->next;
		} else {
			head = curr->next;

			delete curr->value;
			delete curr;
		}
	}	
}