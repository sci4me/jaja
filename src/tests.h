#ifdef TESTING

#ifndef TESTS_H
#define TESTS_H

#include "types.h"

typedef void (*test_fn)();

struct TestCase {
	const char *name;
	test_fn fn;
};

void __setup_test(TestCase tc);

#define DEFINE_TEST(name) void _##name()

#endif

#endif