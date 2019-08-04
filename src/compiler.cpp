#include "compiler.h"

Value Compiler::compile(Array<Node*>* ast) {
	auto result = heap->alloc();
	
	auto j = jit_init();	
	jit_enable_optimization(j, JIT_OPT_ALL);
	jit_prolog(j, &result.lambda.fn);
	jit_declare_arg(j, JIT_PTR, sizeof(Heap*));
	jit_declare_arg(j, JIT_PTR, sizeof(Scope*));
	jit_declare_arg(j, JIT_PTR, sizeof(Stack*));

	jit_getarg(j, R(0), 0);
	jit_getarg(j, R(1), 1);
	jit_getarg(j, R(2), 2);

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
			jit_prepare(j);
			jit_putargr(j, R(2));
			jit_call(j, __rt_push_true);
			break;
		case AST_CONST_FALSE:
			jit_prepare(j);
			jit_putargr(j, R(2));
			jit_call(j, __rt_push_false);
			break;
		case AST_CONST_NIL:
			jit_prepare(j);
			jit_putargr(j, R(2));
			jit_call(j, __rt_push_nil);
			break;
		case AST_CONST_NUMBER:
			jit_prepare(j);
			jit_putargr(j, R(2));
			jit_putargi(j, n->number);
			jit_call(j, __rt_push_number);
			break;
		case AST_CONST_STRING: {
			jit_op *skip_data = jit_jmpi(j, JIT_FORWARD);
			jit_label *l = jit_get_label(j);
			jit_data_str(j, n->string);
			jit_code_align(j, 32);
			jit_patch(j, skip_data);

			jit_ref_data(j, R(3), l);

			jit_prepare(j);
			jit_putargr(j, R(2));
			jit_putargr(j, R(3));
			jit_call(j, __rt_push_string);
			break;
		}
		case AST_CONST_REFERENCE:
			jit_prepare(j);
			jit_putargr(j, R(2));
			jit_putargi(j, n->number);
			jit_call(j, __rt_push_reference);
			break;
		default:
			break;
	}
}