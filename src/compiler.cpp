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
		switch(node->type) {
			case NODE_LAMBDA:
				compile_lambda(j, static_cast<LambdaNode*>(node));
				break;
			case NODE_INSTRUCTION:
				compile_instruction(j, static_cast<InstructionNode*>(node));
				break;
			case NODE_CONSTANT:
				compile_constant(j, static_cast<ConstantNode*>(node));
				break;
			default:
				assert(false);
		}
	}

	jit_reti(j, 0);

	jit_check_code(j, JIT_WARN_ALL);
	jit_generate_code(j);

	return result;
}	

void Compiler::compile_lambda(jit *j, LambdaNode *n) {
	// we want to generate code that allocates a new Lambda value using the Heap
	// it must have fn set to the correct function pointer
	// There may be multiple lambdas created with the same function pointer so we need to handle that
}

void Compiler::compile_instruction(jit *j, InstructionNode *n) {
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
		default:
			assert(false);
	}
}

void Compiler::compile_constant(jit *j, ConstantNode *n) {
	switch(n->type) {
		case AST_CONST_TRUE:
			break;
		case AST_CONST_FALSE:
			break;
		case AST_CONST_NIL:
			break;
		case AST_CONST_NUMBER:
			break;
		case AST_CONST_STRING:
			break;
		case AST_CONST_REFERENCE:
			break;
		default:
			break;
	}
}