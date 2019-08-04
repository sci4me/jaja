#include <stdio.h>

#include "types.h"
#include "utils.h"
#include "parser.h"
#include "compiler.h"
#include "gc.h"

s32 main(s32 argc, char **argv) {
	if(argc != 2) {
		fprintf(stderr, "Usage: %s <file>\n", argv[0]);
		return 1;
	}

	char *source = read_entire_file(argv[1]);
	if(!source) {
		fprintf(stderr, "Unable to read file: %s\n", argv[1]);
		return 1;
	}

	auto p = Parser(argv[1], source);
	auto ast = p.parse();

	auto heap = Heap();
	auto compiler = Compiler(&heap);
	
	auto main = compiler.compile(ast);
	
	auto G = Scope();
	auto stack = Stack();
	
	(*main.lambda.fn)(&heap, &G, &stack);

	return 0;
}