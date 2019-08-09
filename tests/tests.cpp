#ifdef TESTING

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>

#include "tests.h"

static TestCase tests[MAX_TESTS];
static u32 test_count;

extern void test_setup();

void __setup_test(const char *name, test_fn fn) {
	tests[test_count++] = { .name = name, .fn = fn };
}

s32 main(s32 argc, char **argv) {
	test_setup();

	u32 failed = 0;
	u32 succeeded = 0;

	for(u32 i = 0; i < test_count; i++) {
		auto test = tests[i];

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

	printf(
		"\nFailed: %u (%.2f%)\nPassed: %u (%.2f%)\nTotal:  %u\n", 
		failed, 
		((f64)failed / (f64)test_count) * 100.0f, 
		succeeded, 
		((f64)succeeded / (f64)test_count) * 100.0f, 
		test_count);

	return 0;
}

#endif