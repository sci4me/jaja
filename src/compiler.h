#ifndef COMPILER_H
#define COMPILER_H

#include "ast.h"
#include "runtime.h"
#include "gc.h"
#include "bitset.h"

// #define JIT_DEBUG

#define jit_call_method(j, m) { auto fptr = m; jit_call(j, reinterpret_cast<void *&>(fptr)); }

#define R_HEAP R(0)
#define R_SCOPE R(1)
#define R_STACK R(2)

struct Compiler {
	Heap *heap;

	Bitset *registers;
	Array<Bitset*> registersStack;

	Compiler(Heap *_heap) : heap(_heap), registers(0) {};

	jit_value ralloc();
	void rfree(jit_value r);

	Value compile(Array<Node*>* ast);
	Lambda compile_raw(Array<Node*>* ast);
	void compile_lambda(jit *j, Node *n);
	void compile_instruction(jit *j, Node *n);
	void compile_constant(jit *j, Node *n);
};

#endif