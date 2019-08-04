#ifndef COMPILER_H
#define COMPILER_H

#include "ast.h"
#include "runtime.h"
#include "gc.h"

struct Compiler {
	Heap *heap;

	Compiler(Heap *_heap) : heap(_heap) {}

	Value* compile(Array<Node*>* ast);
};

#endif