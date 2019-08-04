#ifndef COMPILER_H
#define COMPILER_H

#include "ast.h"
#include "runtime.h"
#include "gc.h"

struct Compiler {
	Heap *heap;

	Compiler(Heap *_heap) : heap(_heap) {}

	Value* compile(Array<Node*>* ast);
	void compile_lambda(jit *j, LambdaNode *n);
	void compile_instruction(jit *j, InstructionNode *n);
	void compile_constant(jit *j, ConstantNode *n);
};

#endif