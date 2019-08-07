#ifndef ARENA_H
#define ARENA_H

#include "types.h"
#include "allocator.h"

struct Block {
	Block *next;
	u8 *data;
	u64 used;
};

struct Arena {
	Block *head;
	u64 block_size;
	u64 block_count;

	Arena(u64 block_size = 0x10000);
	~Arena();

	u8* alloc(u64 size);
	void reset();

	Allocator as_allocator();
};

#endif