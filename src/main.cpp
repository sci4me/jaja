#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "utils.h"
#include "parser.h"
#include "compiler.h"
#include "gc.h"
#include "arena.h"
#include "optimizer.h"

#define ADD_STD_FN(g, func, name) { Value v; v.a = 0; v.type = VALUE_NATIVE; v.native = func; g.set((char*)name, &v); }

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

	auto parser_arena = new Arena();
	auto p = Parser(parser_arena->as_allocator(), argv[1], source);
	auto ast = p.parse();
	free(source);

	optimize(parser_arena->as_allocator(), ast);

	// FOR(ast, i) {
		// ast->data[i]->print_as_bytecode();
	// }
	// printf("\n");

	auto heap = Heap();
	auto compiler = Compiler();
	compiler.start();
	auto main = compiler.compile(ast);
	compiler.end();
	// delete parser_arena; // TODO

	auto G = Scope(&heap);

	ADD_STD_FN(G, __std_print, "print");
	ADD_STD_FN(G, __std_println, "println");

	auto stack = Stack(&heap);

	// printf("%p %p %p\n", &heap, &G, &stack);

	// (*main.lambda)(&heap, &G, &stack);

	auto c = (lambda_fn) jit_function_to_closure(main.lambda.j);
	(*c)(&heap, &G, &stack);

	// just for fun
	while(stack.data.count) stack.pop();
	G.pop(&heap);
	heap.gc();

	assert(heap.allocations == 0);
	assert(heap.roots.count == 0);

	/*
	FOR((&heap.allocations), i) {
		auto a = heap.allocations.data[i];
		printf("root (%p) : %s@%s:%u\n", a, a->func, a->file, a->line);
	}
	*/

	delete parser_arena;

	return 0;
}