#include <assert.h>
#include <stdio.h>

#include "arena.h"

Arena::Arena(Allocator _allocator, u64 _block_size) : allocator(_allocator), block_size(_block_size), block_count(1) {
	head = (Block*) ALLOC(allocator, sizeof(Block));
	head->data = (u8*) ALLOC(allocator, _block_size);
	head->next = NULL;
	head->used = 0;
}

Arena::~Arena() {
	auto curr = head;
	while(curr) {
		auto next = curr->next;

		FREE(allocator, curr->data);
		FREE(allocator, curr);

		curr = next;
	}
}

u8* Arena::alloc(u64 n) {
	// TODO: alignment
	
	auto left = block_size - head->used;
	if(n > left) {
		auto next = (Block*) ALLOC(allocator, sizeof(Block));
		next->data = (u8*) ALLOC(allocator, block_size);
		next->next = head;
		next->used = 0;
		head = next;
		block_count++;
	}

	auto p = head->data + head->used;
	head->used += n;
	return p;
}

static void* __alloc(void *data, unsigned long int n) {
	return static_cast<Arena*>(data)->alloc(n);
}

static void __free(void *data, void *) {}

Allocator Arena::as_allocator() {
	return {
		.data = this,
		.alloc = __alloc,
		.free = __free
	};
}