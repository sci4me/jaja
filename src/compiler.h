#ifndef COMPILER_H
#define COMPILER_H

#include "ast.h"
#include "runtime.h"
#include "gc.h"

#define JIT_DEBUG

struct Compiler {
	Heap *heap;

	Compiler(Heap *_heap) : heap(_heap) {}

	Value compile(Array<Node*>* ast);
	Lambda compile_raw(Array<Node*>* ast);
	void compile_lambda(jit *j, Node *n);
	void compile_instruction(jit *j, Node *n);
	void compile_constant(jit *j, Node *n);
};

#endif