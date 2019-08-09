#ifndef TESTS_H
#define TESTS_H

#include "../src/types.h"

#define MAX_TESTS 1024

typedef void (*test_fn)();

struct TestCase {
	const char *name;
	test_fn fn;
};

void __setup_test(TestCase tc);

#define DEFINE_TEST(name) void _##name()

#endif