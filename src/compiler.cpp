#include "compiler.h"

Value* Compiler::compile(Array<Node*>* ast) {
	auto result = heap->alloc();
	
	auto j = jit_init();	
	jit_enable_optimization(j, JIT_OPT_ALL);
	jit_prolog(j, &result->lambda.fn);
	jit_declare_arg(j, JIT_PTR, sizeof(Scope*));
	jit_declare_arg(j, JIT_PTR, sizeof(Stack*));

	FOR(ast, i) {
		auto node = ast->data[i];
		
	}

	jit_reti(j, 0);

	jit_check_code(j, JIT_WARN_ALL);
	jit_generate_code(j);

	return result;
}	