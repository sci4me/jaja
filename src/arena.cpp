#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "arena.h"

Arena::Arena(u64 _block_size) : block_size(_block_size), block_count(1) {
	head = (Block*) malloc(sizeof(Block));
	head->data = (u8*) malloc(_block_size);
	head->next = NULL;
	head->used = 0;
}

Arena::~Arena() {
	auto curr = head;
	while(curr) {
		auto next = curr->next;

		free(curr->data);
		free(curr);

		curr = next;
	}
}

u8* Arena::alloc(u64 n) {
	auto left = block_size - head->used;
	if(n > left) {
		auto next = (Block*) malloc(sizeof(Block));
		next->data = (u8*) malloc(block_size);
		next->next = head;
		next->used = 0;
		head = next;
		block_count++;
	}

	auto p = head->data + head->used;
	head->used += n;
	return p;
}

void Arena::reset() {
	Block *last;

	for(auto curr = head; curr; curr = curr->next) {
		last = curr;

		curr->used = 0;
	}

	head = last;

	// TODO: are we gonna leak memory here? (yes..)
}