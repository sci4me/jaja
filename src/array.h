#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>
#include <assert.h>

#include "types.h"
#include "allocator.h"

#define FOR(a, i) for(u32 i = 0; i < a->count; i++)

template<typename T>
struct Array {
	Allocator allocator;
	u32 size;
	u32 count;
	T *data;

	Array(Allocator _allocator = cstdlib_allocator) : allocator(_allocator) {
		size = 16;
		count = 0;
		data = NULL;
	}

	~Array() {
		allocator.free(allocator.data, data);
	}

	void clear() {
		count = 0;
	}

	void push(T value) {
		if(data == NULL) {
			data = (T*) allocator.alloc(allocator.data, size * sizeof(T));
			// data = (T*) calloc(size, sizeof(T));
		}

		if(count >= size - 1) {
			size *= 2;
			data = (T*) allocator.realloc(allocator.data, data, size * sizeof(T));
		}

		data[count++] = value;
	}

	T pop() {
		assert(count > 0);
		auto v = data[count - 1];
		count--;
		return v;
	}

	bool unordered_remove(u32 index) {
		if(index >= count) return false;

		auto v = pop();
		if(index < count) {
			data[index] = v;
		}

		return true;
	}

	bool ordered_remove(u32 index) {
		if(index >= count) return false;

		for(u32 i = index; i < count - 1; i++) {
			data[i] = data[i + 1];
		}
		count--;

		return true;
	}

	s64 index_of(T value) {
		for(u32 i = 0; i < count; i++) {
			if(data[i] == value) return i;
		}
		return -1;
	}
};

#endif