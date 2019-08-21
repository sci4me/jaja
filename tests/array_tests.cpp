#include "test.h"
#include "../src/array.h"

DEFINE_TEST(array_insert_before_works) {
	auto a = Array<u32>();

	a.push(3);
	a.push(6);
	a.push(9);

	a.insert_before(0, 4);
	a.insert_before(2, 8);
	a.insert_before(4, 12);

	assert(a.count == 6);
	assert(a.data[0] == 4);
	assert(a.data[1] == 3);
	assert(a.data[2] == 8);
	assert(a.data[3] == 6);
	assert(a.data[4] == 12);
	assert(a.data[5] == 9);	
}

DEFINE_TEST(array_insert_after_works) {
	auto a = Array<u32>();

	a.push(3);
	a.push(6);
	a.push(9);

	a.insert_after(0, 4);
	a.insert_after(2, 8);
	a.insert_after(4, 12);

	assert(a.count == 6);
	assert(a.data[0] = 3);
	assert(a.data[1] == 4);
	assert(a.data[2] == 6);
	assert(a.data[3] == 8);
	assert(a.data[4] == 9);
	assert(a.data[5] == 12);
}

DEFINE_TEST(array_extend_after_works) {
	{
		auto a = Array<u32>();

		a.push(2);
		a.push(4);
		a.push(6);
		a.push(8);
		a.push(10);

		auto b = Array<u32>();

		b.push(3);
		b.push(6);
		b.push(9);

		a.extend_after(2, &b);

		assert(a.count == 8);
		assert(a.data[0] == 2);
		assert(a.data[1] == 4);
		assert(a.data[2] == 6);
		assert(a.data[3] == 3);
		assert(a.data[4] == 6);
		assert(a.data[5] == 9);
		assert(a.data[6] == 8);;
		assert(a.data[7] == 10);
	}

	{
		auto a = Array<u32>();

		a.push(2);
		a.push(4);
		a.push(6);
		a.push(8);
		a.push(10);
	
		auto b = Array<u32>();

		b.push(3);
		b.push(6);
		b.push(9);

		a.extend_after(4, &b);

		assert(a.count == 8);
		assert(a.data[0] == 2);
		assert(a.data[1] == 4);
		assert(a.data[2] == 6);
		assert(a.data[3] == 8);
		assert(a.data[4] == 10);
		assert(a.data[5] == 3);
		assert(a.data[6] == 6);
		assert(a.data[7] == 9);
	}

	{
		auto a = Array<u32>();

		a.push(2);
		a.push(4);
		a.push(6);
		a.push(8);
		a.push(10);

		auto b = Array<u32>();

		b.push(3);
		b.push(6);
		b.push(9);

		a.extend_after(0, &b);

		assert(a.count == 8);
		assert(a.data[0] == 2);
		assert(a.data[1] == 3);
		assert(a.data[2] == 6);
		assert(a.data[3] == 9);
		assert(a.data[4] == 4);
		assert(a.data[5] == 6);
		assert(a.data[6] == 8);
		assert(a.data[7] == 10);
	}
}

DEFINE_TEST(array_push_works) {
	auto a = Array<u32>();
	
	for(u32 i = 0; i < 10; i++) a.push(i);

	assert(a.count == 10);
	for(u32 i = 0; i < 10; i++) assert(a.data[i] == i);
}

DEFINE_TEST(array_pop_works) {
	auto a = Array<u32>();

	for(u32 i = 0; i < 10; i++) a.push(i);

	for(u32 i = 0; i < 10; i++) assert(a.pop() == 9 - i);
}

DEFINE_TEST(array_clear_works) {
	auto a = Array<u32>();

	a.push(42);
	a.push(69);

	assert(a.count == 2);

	a.clear();

	assert(a.count == 0);
}

DEFINE_TEST(array_unordered_remove_works) {
	auto a = Array<u32>();

	a.push(1);
	a.push(2);
	a.push(3);
	a.push(4);
	a.push(5);

	assert(!a.unordered_remove(5));

	assert(a.unordered_remove(1));
	assert(a.unordered_remove(3));

	assert(a.count == 3);

	assert(a.data[0] == 1);
	assert(a.data[1] == 5);
	assert(a.data[2] == 3);
}

DEFINE_TEST(array_ordered_remove_works) {
	auto a = Array<u32>();

	a.push(1);
	a.push(2);
	a.push(3);
	a.push(4);
	a.push(5);

	assert(!a.ordered_remove(5));

	assert(a.ordered_remove(1));
	assert(a.ordered_remove(3));

	assert(a.count == 3);

	assert(a.data[0] == 1);
	assert(a.data[1] == 3);
	assert(a.data[2] == 4);
}

DEFINE_TEST(array_index_of_works) {
	auto a = Array<u32>();

	for(u32 i = 0; i < 5; i++) a.push(4 - i);

	for(u32 i = 0; i < 5; i++) assert(a.index_of(i) == 4 - i);

	assert(a.index_of(42) == -1);
}

DEFINE_TEST(array_expands_as_needed) {
	auto a = Array<u32>();

	auto n = a.size;

	for(u32 i = 0; i < n; i++) a.push(i);

	assert(a.size == n * 2);
	for(u32 i = 0; i < n; i++) assert(a.data[i] == i);
}

DEFINE_TEST(array_for_macro_works) {
	auto a = Array<u32>();

	for(u32 i = 0; i < 5; i++) a.push(i);

	FOR((&a), i) {
		assert(a.data[i] == i);
	}
}

DEFINE_TEST(array_stress_test) {
	auto a = Array<u32>();

	const u32 n = 10000;

	for(u32 i = 0; i < n; i++) a.push(i * 3);

	assert(a.count == n);

	for(u32 i = 0; i < n; i++) assert(a.data[i] == i * 3);
}