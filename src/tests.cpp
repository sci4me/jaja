#ifdef TESTING

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "tests.h"
#include "array.h"

static Array<TestCase> tests;

extern void test_setup();

void __setup_test(const char *name, test_fn fn) {
	tests.push({ .name = name, .fn = fn });
}

s32 main(s32 argc, char **argv) {
	test_setup();

	u32 failed = 0;
	u32 succeeded = 0;

	FOR((&tests), i) {
		auto test = tests.data[i];

		pid_t pid = fork();
		if(pid == 0) {
			test.fn();
			exit(0);
		} else {
			int status;
			pid_t cpid;
			assert((cpid = wait(&status)) == pid);
			if(status) {
				printf("\u001b[31;1m*\u001b[0m %s\n", test.name);
				failed++;
			} else {
				printf("\u001b[32;1m\u2713\u001b[0m %s\n", test.name);
				succeeded++;
			}
		}
	}

	auto total = tests.count;
	printf(
		"\nFailed: %u (%.2f%)\nPassed: %u (%.2f%)\nTotal:  %u\n", 
		failed, 
		((f64)failed / (f64)total) * 100.0f, 
		succeeded, 
		((f64)succeeded / (f64)total) * 100.0f, 
		total);

	return 0;
}

#endif