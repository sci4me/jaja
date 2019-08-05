#include "compiler.h"

Value Compiler::compile(Array<Node*>* ast) {
	auto result = heap->alloc();
	result.type = VALUE_LAMBDA;
	result.lambda = compile_raw(ast);
	return result;
}

Lambda Compiler::compile_raw(Array<Node*>* ast) {	
	Lambda result;

	auto j = jit_init();	
	result.j = j;

#ifdef DEBUG
	jit_comment(j, ">>>");
#endif

	jit_enable_optimization(j, JIT_OPT_ALL);
	jit_prolog(j, &result.fn);
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

#ifdef DEBUG
	jit_comment(j, "<<<");
#endif

	jit_check_code(j, JIT_WARN_ALL);
	jit_generate_code(j);

#ifdef DEBUG
	printf("ops {\n");
	jit_dump_ops(j, JIT_DEBUG_OPS);
	printf("}\n");
#endif

	return result;
}

void Compiler::compile_lambda(jit *j, LambdaNode *n) {
	auto l = compile_raw(&n->body);

#ifdef DEBUG
	jit_comment(j, "__rt_push_lambda");
#endif

	jit_prepare(j);
	jit_putargr(j, R(2));
	jit_putargr(j, R(0));
	jit_putargi(j, l.j);
	jit_putargi(j, l.fn);
	jit_call(j, __rt_push_lambda);
}

#define xstr(a) str(a)
#define str(a) #a

#ifdef DEBUG
	#define JIT_RT_CALL_2(fn) jit_comment(j, str(fn) "\n"); jit_prepare(j); jit_putargr(j, R(2)); jit_call(j, fn);
	#define JIT_RT_CALL_20(fn) jit_comment(j, str(fn) "\n"); jit_prepare(j); jit_putargr(j, R(2)); jit_putargr(j, R(0)); jit_call(j, fn);
	#define JIT_RT_CALL_21(fn) jit_comment(j, str(fn) "\n"); jit_prepare(j); jit_putargr(j, R(2)); jit_putargr(j, R(1)); jit_call(j, fn);
	#define JIT_RT_CALL_012(fn) jit_comment(j, str(fn) "\n"); jit_prepare(j); jit_putargr(j, R(0)); jit_putargr(j, R(1)); jit_putargr(j, R(2)); jit_call(j, fn);
#else
	#define JIT_RT_CALL_2(fn) jit_prepare(j); jit_putargr(j, R(2)); jit_call(j, fn);
	#define JIT_RT_CALL_20(fn) jit_prepare(j); jit_putargr(j, R(2)); jit_putargr(j, R(0)); jit_call(j, fn);
	#define JIT_RT_CALL_21(fn) jit_prepare(j); jit_putargr(j, R(2)); jit_putargr(j, R(1)); jit_call(j, fn);
	#define JIT_RT_CALL_012(fn) jit_prepare(j); jit_putargr(j, R(0)); jit_putargr(j, R(1)); jit_putargr(j, R(2)); jit_call(j, fn);
#endif

void Compiler::compile_instruction(jit *j, InstructionNode *n) {
	switch(n->op) {
		case AST_OP_EQ:
			JIT_RT_CALL_2(__rt_eq);
			break;
		case AST_OP_LT:
			JIT_RT_CALL_2(__rt_lt);
			break;
		case AST_OP_GT:
			JIT_RT_CALL_2(__rt_gt);
			break;
		case AST_OP_COND_EXEC:
			JIT_RT_CALL_012(__rt_cond_exec);
			break;
		case AST_OP_EXEC:
			JIT_RT_CALL_012(__rt_exec);
			break;
		case AST_OP_AND:
			JIT_RT_CALL_2(__rt_and);
			break;
		case AST_OP_OR:
			JIT_RT_CALL_2(__rt_or);
			break;
		case AST_OP_NOT:
			JIT_RT_CALL_2(__rt_not);
			break;
		case AST_OP_ADD:
			JIT_RT_CALL_2(__rt_add);
			break;
		case AST_OP_SUB:
			JIT_RT_CALL_2(__rt_sub);
			break;
		case AST_OP_MUL:
			JIT_RT_CALL_2(__rt_mul);
			break;
		case AST_OP_DIV:
			JIT_RT_CALL_2(__rt_div);
			break;
		case AST_OP_NEG:
			JIT_RT_CALL_2(__rt_neg);
			break;
		case AST_OP_MOD:
			JIT_RT_CALL_2(__rt_mod);
			break;
		case AST_OP_NEW_OBJECT:
			JIT_RT_CALL_20(__rt_newobj);
			break;
		case AST_OP_GET_PROP:
			JIT_RT_CALL_2(__rt_get_prop);
			break;
		case AST_OP_SET_PROP:
			JIT_RT_CALL_2(__rt_set_prop);
			break;
		case AST_OP_DUP:
			JIT_RT_CALL_2(__rt_dup);
			break;
		case AST_OP_DROP:
			JIT_RT_CALL_2(__rt_drop);
			break;
		case AST_OP_SWAP:
			JIT_RT_CALL_2(__rt_swap);
			break;
		case AST_OP_ROT:
			JIT_RT_CALL_2(__rt_rot);
			break;
		case AST_OP_LOAD:
			JIT_RT_CALL_21(__rt_load);
			break;
		case AST_OP_STORE:
			JIT_RT_CALL_21(__rt_store);
			break;
		case AST_OP_WHILE:
			JIT_RT_CALL_2(__rt_while);
			break;
		default:
			assert(false);
	}
}

void Compiler::compile_constant(jit *j, ConstantNode *n) {
	switch(n->type) {
		case AST_CONST_TRUE:
			JIT_RT_CALL_2(__rt_push_true);
			break;
		case AST_CONST_FALSE:
			JIT_RT_CALL_2(__rt_push_false);
			break;
		case AST_CONST_NIL:
			JIT_RT_CALL_2(__rt_push_nil);
			break;
		case AST_CONST_NUMBER:
#ifdef DEBUG
			jit_comment(j, "__rt_push_number");
#endif
			jit_prepare(j);
			jit_putargr(j, R(2));
			jit_putargi(j, n->number);
			jit_call(j, __rt_push_number);
			break;
		case AST_CONST_STRING: {
#ifdef DEBUG
			jit_comment(j, "__rt_push_string");
#endif
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
#ifdef DEBUG
			jit_comment(j, "__rt_push_reference");
#endif
			jit_prepare(j);
			jit_putargr(j, R(2));
			jit_putargi(j, n->number);
			jit_call(j, __rt_push_reference);
			break;
		default:
			break;
	}
}