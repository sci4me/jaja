#ifndef TESTS_H
#define TESTS_H

#include "../src/types.h"

#define MAX_FILES 128
#define MAX_TESTS MAX_FILES * 128

typedef void (*test_fn)();

struct TestCase {
	u32 file_index;
	const char *name;
	test_fn fn;
};

void __setup_test(const char *file, const char *name, test_fn fn);

#define DEFINE_TEST(name) void _##name()

#endif