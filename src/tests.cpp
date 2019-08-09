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

	printf("Running %u tests...\n", tests.count);

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
				printf("\u001b[31;1m*\u001b[0m %s failed\n", test.name);
			} else {
				printf("\u001b[32;1m\u2713\u001b[0m %s succeeded\n", test.name);
			}
		}
	}

	return 0;
}

#endif