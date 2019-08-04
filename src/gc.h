#ifndef GC_H
#define GC_H

#include "runtime.h"

struct Allocation {
	Allocation *next;
	bool marked;
	Value value;
};

struct Heap {
	Allocation *head;
	Array<Value> roots;

	Value alloc();
	void mark_root(Value v);
	void unmark_root(Value v);
	void gc();

	void mark(Value value);
	void sweep();
};

#endif