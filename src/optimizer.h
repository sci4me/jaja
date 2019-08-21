#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "array.h"
#include "ast.h"
#include "allocator.h"

void optimize(Allocator allocator, Array<Node*> *code);

#endif