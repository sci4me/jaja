#ifndef ARRAY_H
#define ARRAY_H

#include <assert.h>
#include <string.h>

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
		data = 0;
	}

	~Array() {
		if(data) {
			FREE(allocator, data);
		}
	}

	void clear() {
		count = 0;
	}

	void push(T value) {
		if(data == 0) {
			data = (T*) ALLOC(allocator, size * sizeof(T));
		}

		if(count >= size - 1) {
			auto old_size = size;
			size *= 2;
			auto new_data = (T*) ALLOC(allocator, size * sizeof(T));
			memcpy(new_data, data, old_size * sizeof(T));
			FREE(allocator, data);
			data = new_data;
		}

		data[count++] = value;
	}

	T pop() {
		assert(count > 0);
		auto v = data[count - 1];
		count--;
		return v;
	}

	void drop() {
		assert(count > 0);
		count--;
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
		if(count == 0) return -1;

		for(u32 i = 0; i < count; i++) {
			if(data[i] == value) return i;
		}
		return -1;
	}
};

#endif