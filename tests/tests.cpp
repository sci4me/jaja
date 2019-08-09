#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>

#include "tests.h"

static const char *files[MAX_FILES];
static u32 file_count;
static TestCase tests[MAX_TESTS];
static u32 test_count;

extern void test_setup();

static u32 __test_file_index(const char *file) {
	for(u32 i = 0; i < file_count; i++) {
		if(strcmp(file, files[i]) == 0) return i;
	}

	assert(file_count < MAX_FILES);
	files[file_count] = file;
	return file_count++;
}

void __setup_test(const char *file, const char *name, test_fn fn) {
	auto file_index = __test_file_index(file);
	files[file_index] = file;
	assert(test_count < MAX_TESTS);
	tests[test_count++] = { .file_index = file_index, .name = name, .fn = fn };
}

s32 main(s32 argc, char **argv) {
	test_setup();

	u32 failed = 0;
	u32 passed = 0;

	for(u32 i = 0; i < file_count; i++) {
		auto file = files[i];

		printf("%s:\n", file);

		for(u32 j = 0; j < test_count; j++) {
			auto test = tests[j];

			if(test.file_index != i) continue;

			pid_t pid = fork();
			if(pid == 0) {
				test.fn();
				exit(0);
			} else {
				int status;
				pid_t cpid;
				assert((cpid = wait(&status)) == pid);
				if(status) {
					printf(" \u001b[31;1m*\u001b[0m %s\n", test.name);
					failed++;
				} else {
					printf(" \u001b[32;1m\u2713\u001b[0m %s\n", test.name);
					passed++;
				}
			}
		}

		printf("\n");
	}

	printf(
		"\nFailed: %u (%0.2f%%)\nPassed: %u (%0.2f%%)\nTotal:  %u\n", 
		failed, 
		((f64)failed / (f64)test_count) * 100.0f, 
		passed, 
		((f64)passed / (f64)test_count) * 100.0f, 
		test_count
	);

	return 0;
}