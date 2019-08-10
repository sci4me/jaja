#include <stdio.h>

#include "types.h"
#include "utils.h"
#include "parser.h"
#include "compiler.h"
#include "gc.h"
#include "arena.h"

#define ADD_STD_FN(g, func, name) { Value v; v.a = 0; v.type = VALUE_NATIVE; v.lambda.fn = func; g.set((char*)name, v); }

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

	auto parser_arena = Arena();
	auto p = Parser(parser_arena.as_allocator(), argv[1], source);
	auto ast = p.parse();
	free(source);

	// FOR(ast, i) {
	// 	ast->data[i]->print_as_bytecode();
	// }
	// printf("\n");

	auto heap = Heap();
	auto compiler = Compiler(&heap);
	
	auto main = compiler.compile(ast);

	heap.mark_root(main.a);

	auto G = Scope(NULL);

	ADD_STD_FN(G, __std_print, "print");
	ADD_STD_FN(G, __std_println, "println");

	auto stack = Stack(&heap);

	(*main.lambda.fn)(&heap, &G, &stack);

	// just for fun
	while(stack.data.count) stack.pop();
	G.pop(&heap);
	heap.unmark_root(main.a);
	heap.gc();

	/*
	FOR((&heap.allocations), i) {
		auto a = heap.allocations.data[i];
		printf("root (%p) : %s@%s:%u\n", a, a->func, a->file, a->line);
	}
	*/

	return 0;
}