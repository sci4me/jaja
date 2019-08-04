#ifndef COMPILER_H
#define COMPILER_H

#include "ast.h"
#include "runtime.h"

struct Compiler {
	Lambda* compile(Array<Node*>* ast);
};

#endif