#include "compiler.h"

/*

So, we want to change the way scopes work. We don't want to have a Scope structure at runtime _at all_.
We should allocate variables on the stack and write to them via that pointer when storing to them, and
  read from that pointer when loading them.
Challenge: what if we want to access a variable in a scope about ours? How do we facilitate that?
  It's tricky because we can't use registers because a, that's not what they're meant to be used for, b, the data
  wouldn't fit, and c, each functi n has its own register set.
We need to like, know which variab es need to be passed as parameters or something
But we can't just do that right?
Hmmmhmhmmhmh

Yeah it's surprisingly tricky.

We'd need to know which variables  eed to be passed in from the caller and actually pass them in whenever
  we call that lambda. Can we do t is? We may have to resort to a multi-pass system where we first compute scoping
  information and then generate code using it...?

					- sci4me, Aug 19, 2019

*/

Compiler::Compiler() {
	ctx = jit_context_create();
}

Compiler::~Compiler() {
	jit_context_destroy(ctx);
}

Value Compiler::compile(Array<Node*> *ast) {
	jit_context_build_start(ctx);
	Value result;
	result.a = NULL;
	result.type = VALUE_LAMBDA;
	// result.lambda = compile_raw(ast); TODO
	jit_context_build_end(ctx);
	return result;
}

jit_function_t Compiler::compile_raw(Array<Node*> *ast) {
	auto j = jit_function_create(ctx, lambda_fn_signature);

	FOR(ast, i) {
		auto node = ast->data[i];
		switch(node->type) {
			case NODE_LAMBDA:
				compile_lambda(j, node);
				break;
			case NODE_TRUE:
			case NODE_FALSE:
			case NODE_NIL:
			case NODE_NUMBER:
			case NODE_STRING:
			case NODE_REFERENCE:
				compile_constant(j, node);
				break;
			default:
				compile_instruction(j, node, ast, i);
				break;
		}
	}

	return j;
}

void Compiler::compile_lambda(jit_function_t j, Node *n) {
	auto l = compile(&n->lambda);
	
	// TODO
}

void Compiler::compile_instruction(jit_function_t j, Node *n, Array<Node*> *ast, u32 index) {
	switch(n->op) {
		case AST_OP_EQ:
			break;
		case AST_OP_LT:
			break;
		case AST_OP_GT:
			break;
		case AST_OP_COND_EXEC:
			break;
		case AST_OP_EXEC:
			break;
		case AST_OP_AND:
			break;
		case AST_OP_OR:
			break;
		case AST_OP_NOT:
			break;
		case AST_OP_ADD:
			break;
		case AST_OP_SUB:
			break;
		case AST_OP_MUL:
			break;
		case AST_OP_DIV:
			break;
		case AST_OP_NEG:
			break;
		case AST_OP_MOD:
			break;
		case AST_OP_NEW_OBJECT:
			break;
		case AST_OP_GET_PROP:
			break;
		case AST_OP_SET_PROP:
			break;
		case AST_OP_DUP:
			break;
		case AST_OP_DROP:
			break;
		case AST_OP_SWAP:
			break;
		case AST_OP_ROT:
			break;
		case AST_OP_LOAD:
			break;
		case AST_OP_STORE:
			break;
		case AST_OP_WHILE:
			break;
		case AST_OP_BRANCH:
			break;
		case AST_OP_BRANCH_IF_FALSE:
			break;
		case AST_OP_BRANCH_IF_TRUE:
			break;
		case AST_OP_BRANCH_TARGET:
			break;
		default:
			assert(false);
	}
}

void Compiler::compile_constant(jit_function_t j, Node *n) {
	switch(n->type) {
		case NODE_TRUE: {
			break;
		}
		case NODE_FALSE: {
			break;
		}
		case NODE_NIL: {
			break;
		}
		case NODE_NUMBER: {
			break;
		}
		case NODE_STRING: {
			break;
		}
		case NODE_REFERENCE: {
			break;
		}
		default:
			assert(false);
			break;
	}
}