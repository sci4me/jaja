#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>

#include "types.h"

#define FOR(a, i) for(u32 i = 0; i < a->count; i++)

template<typename T>
struct Array {
	u32 size;
	u32 count;
	T *data;

	Array() {
		size = 16;
		count = 0;
		data = 0;
	}

	Array(u64 capacity) {
		size = capacity;
		count = 0;
		data = (T*) calloc(size, sizeof(T));
	}

	~Array() {
		free(data);
	}

	void clear() {
		count = 0;
	}

	void add(T value) {
		if(data == 0) {
			data = (T*) calloc(size, sizeof(T));
		}

		if(count >= size - 1) {
			u32 old_size = size;
			size *= 2;
			data = (T*) realloc(data, size * sizeof(T));
		}

		data[count++] = value;
	}

	bool unordered_remove(u32 index) {
		if(index >= count) return false;

		auto v = data[count - 1];
		count--;

		if(index < count) {
			auto tmp = data[index];
			data[index]  = v;
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