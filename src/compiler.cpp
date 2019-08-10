#include "compiler.h"

Value Compiler::compile(Array<Node*>* ast) {
	auto result = GC_ALLOC(heap);
	result->type = VALUE_LAMBDA;
	result->lambda = compile_raw(ast);
	return *result;
}

Lambda Compiler::compile_raw(Array<Node*>* ast) {	
	Lambda result;

	auto j = jit_init();	
	result.j = j;

#ifdef JIT_DEBUG
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
				compile_instruction(j, node);
				break;
		}
	}

	jit_reti(j, 0);

#ifdef JIT_DEBUG
	jit_comment(j, "<<<");
	jit_check_code(j, JIT_WARN_ALL);
#endif

	jit_generate_code(j);

#ifdef JIT_DEBUG	
	printf("ops {\n");
	jit_dump_ops(j, JIT_DEBUG_OPS);
	printf("}\n");
#endif

	return result;
}

void Compiler::compile_lambda(jit *j, Node *n) {
	auto l = compile_raw(&n->lambda);

#ifdef JIT_DEBUG
	jit_comment(j, "__rt_push_lambda");
#endif

	jit_movi(j, R(3), 0);
	jit_prepare(j);
	jit_putargr(j, R(0));
	jit_putargr(j, R(3));
	jit_putargr(j, R(3));
	jit_putargr(j, R(3));
	jit_call_method(j, &Heap::alloc);
	jit_retval(j, R(3));

	jit_movi(j, R(4), VALUE_LAMBDA);
	jit_stxi(j, offsetof(Value, type), R(3), R(4), sizeof(Value::type));
	jit_movi(j, R(4), l.j);
	jit_stxi(j, offsetof(Value, lambda.j), R(3), R(4), sizeof(Value::lambda.j));
	jit_movi(j, R(4), l.fn);
	jit_stxi(j, offsetof(Value, lambda.fn), R(3), R(4), sizeof(Value::lambda.fn));

	jit_prepare(j);
	jit_putargr(j, R(2));
	jit_putargr(j, R(3));
	jit_call_method(j, &Stack::push);
}

#define xstr(a) str(a)
#define str(a) #a

#ifdef JIT_DEBUG
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

void Compiler::compile_instruction(jit *j, Node *n) {
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
			JIT_RT_CALL_012(__rt_store);
			break;
		case AST_OP_WHILE:
			JIT_RT_CALL_012(__rt_while);
			break;
		default:
			assert(false);
	}
}

void Compiler::compile_constant(jit *j, Node *n) {
	switch(n->type) {
		case NODE_TRUE: {
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_push_true");
#endif
			auto i = jit_allocai(j, sizeof(Value));

			jit_addi(j, R(3), R_FP, i);
			jit_movi(j, R(4), 0);
			jit_stxi(j, offsetof(Value, a), R(3), R(4), sizeof(Value::a));
			jit_movi(j, R(4), VALUE_TRUE);
			jit_stxi(j, offsetof(Value, type), R(3), R(4), sizeof(Value::type));

			jit_prepare(j);
			jit_putargr(j, R(2));
			jit_putargr(j, R(3));
			jit_call_method(j, &Stack::push);
			break;
		}
		case NODE_FALSE: {
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_push_false");
#endif
			auto i = jit_allocai(j, sizeof(Value));

			jit_addi(j, R(3), R_FP, i);
			jit_movi(j, R(4), 0);
			jit_stxi(j, offsetof(Value, a), R(3), R(4), sizeof(Value::a));
			jit_movi(j, R(4), VALUE_FALSE);
			jit_stxi(j, offsetof(Value, type), R(3), R(4), sizeof(Value::type));

			jit_prepare(j);
			jit_putargr(j, R(2));
			jit_putargr(j, R(3));
			jit_call_method(j, &Stack::push);
			break;
		}
		case NODE_NIL: {
			#ifdef JIT_DEBUG
			jit_comment(j, "__rt_push_nil");
#endif
			auto i = jit_allocai(j, sizeof(Value));

			jit_addi(j, R(3), R_FP, i);
			jit_movi(j, R(4), 0);
			jit_stxi(j, offsetof(Value, a), R(3), R(4), sizeof(Value::a));
			jit_movi(j, R(4), VALUE_NIL);
			jit_stxi(j, offsetof(Value, type), R(3), R(4), sizeof(Value::type));

			jit_prepare(j);
			jit_putargr(j, R(2));
			jit_putargr(j, R(3));
			jit_call_method(j, &Stack::push);
			break;
		}
		case NODE_NUMBER: {
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_push_number");
#endif
			auto i = jit_allocai(j, sizeof(Value));

			jit_addi(j, R(3), R_FP, i);
			jit_movi(j, R(4), 0);
			jit_stxi(j, offsetof(Value, a), R(3), R(4), sizeof(Value::a));
			jit_movi(j, R(4), VALUE_NUMBER);
			jit_stxi(j, offsetof(Value, type), R(3), R(4), sizeof(Value::type));
			jit_movi(j, R(4), n->number);
			jit_stxi(j, offsetof(Value, number), R(3), R(4), sizeof(Value::number));

			jit_prepare(j);
			jit_putargr(j, R(2));
			jit_putargr(j, R(3));
			jit_call_method(j, &Stack::push);
			break;
		}
		case NODE_STRING: {
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_push_string");
#endif
			jit_op *skip = jit_jmpi(j, JIT_FORWARD);

			jit_label *l = jit_get_label(j);
			jit_data_str(j, n->string);

			jit_code_align(j, 32);
			jit_patch(j, skip);

			jit_ref_data(j, R(3), l);

			auto i = jit_allocai(j, sizeof(Value));

			jit_addi(j, R(4), R_FP, i);
			jit_movi(j, R(5), 0);
			jit_stxi(j, offsetof(Value, a), R(4), R(5), sizeof(Value::a));
			jit_movi(j, R(5), VALUE_STRING);
			jit_stxi(j, offsetof(Value, type), R(4), R(5), sizeof(Value::type));
			jit_stxi(j, offsetof(Value, string), R(4), R(3), sizeof(Value::string));

			jit_prepare(j);
			jit_putargr(j, R(2));
			jit_putargr(j, R(4));
			jit_call_method(j, &Stack::push);
			break;
		}
		case NODE_REFERENCE: {
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_push_reference");
#endif
			jit_op *skip = jit_jmpi(j, JIT_FORWARD);

			jit_label *l = jit_get_label(j);
			jit_data_str(j, n->string);

			jit_code_align(j, 32);
			jit_patch(j, skip);

			jit_ref_data(j, R(3), l);

			auto i = jit_allocai(j, sizeof(Value));

			jit_addi(j, R(4), R_FP, i);
			jit_movi(j, R(5), 0);
			jit_stxi(j, offsetof(Value, a), R(4), R(5), sizeof(Value::a));
			jit_movi(j, R(5), VALUE_REFERENCE);
			jit_stxi(j, offsetof(Value, type), R(4), R(5), sizeof(Value::type));
			jit_stxi(j, offsetof(Value, string), R(4), R(3), sizeof(Value::string));

			jit_prepare(j);
			jit_putargr(j, R(2));
			jit_putargr(j, R(4));
			jit_call_method(j, &Stack::push);
			break;
		}
		default:
			assert(false);
			break;
	}
}