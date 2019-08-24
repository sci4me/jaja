#ifndef COMPILER_H
#define COMPILER_H

#include <jit/jit.h>
#include <jit/jit-type.h>

#include "ast.h"
#include "runtime.h"
#include "gc.h"
#include "bitset.h"

struct Compiler {
	jit_context_t ctx;

	jit_type_t lambda_fn_signature;
	jit_type_t stack_push_signature;

	jit_type_t __rt_signature_2;
	jit_type_t __rt_signature_20;
	jit_type_t __rt_signature_21;
	jit_type_t __rt_signature_012;

	Compiler();
	~Compiler();

	void start();
	void end();
	Value compile(Array<Node*> *ast);
	Lambda compile_raw(Array<Node*> *ast);
	void compile_lambda(jit_function_t j, Node *n);
	void compile_instruction(jit_function_t j, Node *n, Array<Node*> *ast, u32 index);
	void compile_constant(jit_function_t j, Node *n);
};

#endif