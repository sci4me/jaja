#include "compiler.h"

Lambda* Compiler::compile(Array<Node*>* ast) {
	// TODO: allocate via our heap w/ GC
	auto result = new Lambda();	

	result->j = jit_init();
	jit_enable_optimization(result->j, JIT_OPT_ALL);
	jit_prolog(result->j, &result->fn);
	jit_declare_arg(result->j, JIT_PTR, sizeof(Scope*));
	jit_declare_arg(result->j, JIT_PTR, sizeof(Stack*));

	

	jit_reti(result->j, 0);

	jit_check_code(result->j, JIT_WARN_ALL);
	jit_generate_code(result->j);

	return result;
}	