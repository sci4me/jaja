#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>

#include "types.h"

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

	~Array() {
		free(data);
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

		data[index] = data[count - 1];
		count--;

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

	u32 index_of(T value) {
		for(u32 i = 0; i < count; i++) {
			if(data[i] == value) return i;
		}
		return -1;
	}

	T operator[](u32 index) const {
		return data[index];
	}

	T& operator[](u32 index) {
		return data[index];
	}
};

#endif