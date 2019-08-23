#include <stdio.h>

#include "types.h"
#include "utils.h"
#include "parser.h"
#include "compiler.h"
#include "gc.h"
#include "arena.h"
#include "optimizer.h"

#define ADD_STD_FN(g, func, name) { Value v; v.a = 0; v.type = VALUE_NATIVE; v.lambda.fn = func; g.set((char*)name, &v); }

#include <stdlib.h>
#include <jit/jit.h>
#include <jit/jit-type.h>
#include <jit/jit-dump.h>

static void compile_and_run(char *source) {
	auto ctx = jit_context_create();
	
	jit_context_build_start(ctx);

	jit_type_t putchar_params[] = { jit_type_int };
	auto putchar_signature = jit_type_create_signature(jit_abi_cdecl, jit_type_int, putchar_params, 1, 0);
	auto getchar_signature = jit_type_create_signature(jit_abi_cdecl, jit_type_int, 0, 0, 0);

	auto signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, 0, 0, 0);
	auto j = jit_function_create(ctx, signature);

	auto tapeSize = jit_value_create_nint_constant(j, jit_type_int, 30000);
	auto tape = jit_insn_alloca(j, tapeSize);
	auto zero = jit_value_create_nint_constant(j, jit_type_int, 0);
	jit_insn_memset(j, tape, zero, tapeSize);

	auto jit_type_ubyte_ptr = jit_type_create_pointer(jit_type_ubyte, 0);
	
	auto dp = jit_value_create(j, jit_type_ubyte_ptr);
	jit_insn_store(j, dp, tape);

	Array<jit_label_t> loop_starts;
	Array<jit_label_t> loop_ends;

	auto len = strlen(source);
	auto i = 0;
	while(i < len) {
		auto c = source[i];
		switch(c) {
			case '+':
			case '-':
			case '>':
			case '<': {
				auto n = 1;

				{
					auto j = i + 1;
					while(j < len && source[j] == c) {
						j++;
						n++;
					}
					i = j;
				}

				auto count = jit_value_create_nint_constant(j, jit_type_int, n);

				if(c == '+' || c == '-') {
					auto old = jit_insn_load_relative(j, dp, 0, jit_type_ubyte);
					jit_value_t x;
					if(c == '+') {
						x = jit_insn_add(j, old, count);
					} else {
						x = jit_insn_sub(j, old, count);
					}
					auto y = jit_insn_convert(j, x, jit_type_ubyte, 0);
					jit_insn_store_relative(j, dp, 0, y);
				} else {
					jit_value_t x;
					if(c == '>') {
						x = jit_insn_add(j, dp, count);
					} else {
						x = jit_insn_sub(j, dp, count);
					}
					jit_insn_store(j, dp, x);
				}
				break;
			}
			case '.': {
				auto x = jit_insn_load_relative(j, dp, 0, jit_type_ubyte);
				jit_insn_call_native(j, "putchar", (void*)putchar, putchar_signature, &x, 1, JIT_CALL_NOTHROW);				

				i++;
				break;
			}
			case ',': {
				// TODO
				i++;
				break;
			}
			case '[': {
				if(i + 2 < len && source[i + 1] == '-' && source[i + 2] == ']') {
					auto x = jit_insn_convert(j, zero, jit_type_ubyte, 0);
					jit_insn_store_relative(j, dp, 0, x);

					i += 3;
				} else {
					auto x = jit_insn_load_relative(j, dp, 0, jit_type_ubyte);
					auto c = jit_insn_eq(j, x, zero);

					auto end = jit_label_undefined;
					jit_insn_branch_if(j, c, &end);
					loop_ends.push(end);

					jit_label_t start = jit_label_undefined;
					jit_insn_label(j, &start);
					loop_starts.push(start);

					i++;		
				}		
				break;
			}
			case ']': {
				auto start = loop_starts.pop();
				auto end = loop_ends.pop();

				auto x = jit_insn_load_relative(j, dp, 0, jit_type_ubyte);
				auto c = jit_insn_ne(j, x, zero);
				jit_insn_branch_if(j, c, &start);

				jit_insn_label(j, &end);

				i++;
				break;
			}
			default: {
				i++;
				break;
			}
		}
	}

	assert(!loop_starts.count);

	jit_insn_return(j, 0);

	// printf("\n------------------------------------------------------------\n\n");
	// jit_dump_function(stdout, j, "main");
	jit_function_compile(j);
	// printf("------------------------------------------------------------\n\n");
	// jit_dump_function(stdout, j, "main");
	jit_context_build_end(ctx);

	// printf("------------------------------------------------------------\n\n");

	jit_function_apply(j, 0, 0);

	jit_context_destroy(ctx);	
}

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

	compile_and_run(source);
	free(source);

	/*
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
	
	auto main = compiler.compile(ast);
	delete parser_arena;

	auto G = Scope(&heap);

	ADD_STD_FN(G, __std_print, "print");
	ADD_STD_FN(G, __std_println, "println");

	auto stack = Stack(&heap);

	(*main.lambda.fn)(&heap, &G, &stack);

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

	return 0;
}