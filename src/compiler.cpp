#include "compiler.h"

Compiler::Compiler(Heap *_heap) : heap(_heap) {
	registers.set(0);
	registers.set(1);
	registers.set(2);
}

jit_value Compiler::ralloc() {
	u64 n = registers.next_clear();
	registers.set(n);
	return R(n);
}

void Compiler::rfree(jit_value r) {
	jit_reg x;
	memcpy(&x, &r, sizeof(jit_reg));
	auto n = (u64) x.id;

	assert(registers.get(n));
	registers.clear(n);
}

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

	jit_getarg(j, R_HEAP, 0);
	jit_getarg(j, R_SCOPE, 1);
	jit_getarg(j, R_STACK, 2);

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

	auto lambda = ralloc();

#ifdef HEAP_DEBUG
	auto rz = ralloc();
	jit_movi(j, rz, 0);
	jit_prepare(j);
	jit_putargr(j, R_HEAP);
	jit_putargr(j, rz); // TODO put real shit here?
	jit_putargr(j, rz);
	jit_putargr(j, rz);
	jit_call_method(j, &Heap::alloc);
	jit_retval(j, lambda);

	rfree(rz);
#else
	jit_prepare(j);
	jit_putargr(j, R_HEAP);
	jit_call_method(j, &Heap::alloc);
	jit_retval(j, lambda);
#endif

	auto tmp = ralloc();
	jit_movi(j, tmp, VALUE_LAMBDA);
	jit_stxi(j, offsetof(Value, type), lambda, tmp, sizeof(Value::type));
	jit_movi(j, tmp, l.j);
	jit_stxi(j, offsetof(Value, lambda.j), lambda, tmp, sizeof(Value::lambda.j));
	jit_movi(j, tmp, l.fn);
	jit_stxi(j, offsetof(Value, lambda.fn), lambda, tmp, sizeof(Value::lambda.fn));

	jit_prepare(j);
	jit_putargr(j, R_STACK);
	jit_putargr(j, lambda);
	jit_call_method(j, &Stack::push);

	rfree(lambda);
}

#define xstr(a) str(a)
#define str(a) #a

#ifdef JIT_DEBUG
	#define JIT_RT_CALL_2(fn) jit_comment(j, str(fn) "\n"); jit_prepare(j); jit_putargr(j, R_STACK); jit_call(j, fn);
	#define JIT_RT_CALL_20(fn) jit_comment(j, str(fn) "\n"); jit_prepare(j); jit_putargr(j, R_STACK); jit_putargr(j, R_HEAP); jit_call(j, fn);
	#define JIT_RT_CALL_21(fn) jit_comment(j, str(fn) "\n"); jit_prepare(j); jit_putargr(j, R_STACK); jit_putargr(j, R_SCOPE); jit_call(j, fn);
	#define JIT_RT_CALL_012(fn) jit_comment(j, str(fn) "\n"); jit_prepare(j); jit_putargr(j, R_HEAP); jit_putargr(j, R_SCOPE); jit_putargr(j, R_STACK); jit_call(j, fn);
#else
	#define JIT_RT_CALL_2(fn) jit_prepare(j); jit_putargr(j, R_STACK); jit_call(j, fn);
	#define JIT_RT_CALL_20(fn) jit_prepare(j); jit_putargr(j, R_STACK); jit_putargr(j, R_HEAP); jit_call(j, fn);
	#define JIT_RT_CALL_21(fn) jit_prepare(j); jit_putargr(j, R_STACK); jit_putargr(j, R_SCOPE); jit_call(j, fn);
	#define JIT_RT_CALL_012(fn) jit_prepare(j); jit_putargr(j, R_HEAP); jit_putargr(j, R_SCOPE); jit_putargr(j, R_STACK); jit_call(j, fn);
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
		case AST_OP_DUP: {
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_dup");
#endif
			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_call_method(j, &Stack::peek);
			auto tos = ralloc();
			jit_retval(j, tos);

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, tos);
			jit_call_method(j, &Stack::push);
			rfree(tos);
			break;
		}
		case AST_OP_DROP: {
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_drop");
#endif
			auto data = ralloc();
			jit_addi(j, data, R_STACK, offsetof(Stack, data));
			jit_prepare(j);
			jit_putargr(j, data);
			jit_call_method(j, &Array<Value>::drop);
			rfree(data);
			break;
		}
		case AST_OP_SWAP:
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_swap");
#endif
			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_call_method(j, &Stack::swap);
			break;
		case AST_OP_ROT:
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_rot");
#endif
			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_call_method(j, &Stack::rot);
			break;
		case AST_OP_LOAD: {
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_load");
#endif
			auto key = ralloc(); // 3
			jit_addi(j, key, R_FP, jit_allocai(j, sizeof(Value)));

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, key);
			jit_call_method(j, &Stack::pop_into);

#ifndef NDEBUG
			auto type = ralloc();
			jit_ldxi(j, type, key, offsetof(Value, type), sizeof(Value::type));

			auto l = jit_beqi(j, 0, type, VALUE_REFERENCE);
			jit_prepare(j);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_call(j, __assert_fail);
			jit_patch(j, l);
			rfree(type);
#endif

			auto string = ralloc(); // 4
			jit_ldxi(j, string, key, offsetof(Value, string), sizeof(Value::string));

			jit_prepare(j);
			jit_putargr(j, R_SCOPE);
			jit_putargr(j, string);
			jit_call_method(j, &Scope::get); // @Volatile
			auto result = ralloc();
			jit_retval(j, result);

			auto l2 = jit_bnei(j, 0, string, 0);
			auto tmp = ralloc(); // 5
			jit_movi(j, tmp, 0);
			jit_stxi(j, offsetof(Value, a), key, tmp, sizeof(Value::a));
			jit_movi(j, tmp, VALUE_NIL);
			jit_stxi(j, offsetof(Value, type), key, tmp, sizeof(Value::type));
			jit_movr(j, result, key);
			jit_patch(j, l2);

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, result);
			jit_call_method(j, &Stack::push);
			rfree(key);
			rfree(string);
			rfree(result);
			break;
		}
		case AST_OP_STORE: {
			JIT_RT_CALL_012(__rt_store);
			/*
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_store");
#endif

			auto const R_KEY = R(3);
			auto const R_VALUE = R(4);
			jit_addi(j, R_KEY, R_FP, jit_allocai(j, sizeof(Value)));
			jit_addi(j, R_VALUE, R_FP, jit_allocai(j, sizeof(Value)));

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, R_KEY);
			jit_call_method(j, &Stack::pop_into);

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, R_VALUE);
			jit_call_method(j, &Stack::pop_into);			

#ifndef NDEBUG
			jit_ldxi(j, R(5), R_KEY, offsetof(Value, type), sizeof(Value::type));

			auto l = jit_beqi(j, 0, R(5), VALUE_REFERENCE);
			jit_prepare(j);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_call(j, __assert_fail);
			jit_patch(j, l);
#endif	

			jit_ldxi(j, R(5), R_KEY, offsetof(Value, string), sizeof(Value::string));

			jit_prepare(j);
			jit_putargr(j, R_SCOPE);
			jit_putargr(j, R(5));
			jit_putargr(j, R_VALUE);
			jit_call_method(j, &Scope::set);
			*/
			break;
		}
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
			auto v = ralloc();
			jit_addi(j, v, R_FP, jit_allocai(j, sizeof(Value)));
			auto tmp = ralloc();
			jit_movi(j, tmp, 0);
			jit_stxi(j, offsetof(Value, a), v, tmp, sizeof(Value::a));
			jit_movi(j, tmp, VALUE_TRUE);
			jit_stxi(j, offsetof(Value, type), v, tmp, sizeof(Value::type));

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, v);
			jit_call_method(j, &Stack::push);

			rfree(v);
			rfree(tmp);
			break;
		}
		case NODE_FALSE: {
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_push_false");
#endif
			auto i = jit_allocai(j, sizeof(Value));

			auto v = ralloc();
			jit_addi(j, v, R_FP, i);
			auto tmp = ralloc();
			jit_movi(j, tmp, 0);
			jit_stxi(j, offsetof(Value, a), v, tmp, sizeof(Value::a));
			jit_movi(j, tmp, VALUE_FALSE);
			jit_stxi(j, offsetof(Value, type), v, tmp, sizeof(Value::type));

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, v);
			jit_call_method(j, &Stack::push);

			rfree(v);
			rfree(tmp);
			break;
		}
		case NODE_NIL: {
			#ifdef JIT_DEBUG
			jit_comment(j, "__rt_push_nil");
#endif
			auto i = jit_allocai(j, sizeof(Value));

			auto v = ralloc();
			jit_addi(j, v, R_FP, i);
			auto tmp = ralloc();
			jit_movi(j, tmp, 0);
			jit_stxi(j, offsetof(Value, a), v, tmp, sizeof(Value::a));
			jit_movi(j, tmp, VALUE_NIL);
			jit_stxi(j, offsetof(Value, type), v, tmp, sizeof(Value::type));

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, v);
			jit_call_method(j, &Stack::push);

			rfree(v);
			rfree(tmp);
			break;
		}
		case NODE_NUMBER: {
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_push_number");
#endif
			auto v = ralloc();
			jit_addi(j, v, R_FP, jit_allocai(j, sizeof(Value)));
			auto tmp = ralloc();
			jit_movi(j, tmp, 0);
			jit_stxi(j, offsetof(Value, a), v, tmp, sizeof(Value::a));
			jit_movi(j, tmp, VALUE_NUMBER);
			jit_stxi(j, offsetof(Value, type), v, tmp, sizeof(Value::type));
			jit_movi(j, tmp, n->number);
			jit_stxi(j, offsetof(Value, number), v, tmp, sizeof(Value::number));

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, v);
			jit_call_method(j, &Stack::push);

			rfree(v);
			rfree(tmp);
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

			auto str = ralloc();
			jit_ref_data(j, str, l);

			auto v = ralloc();
			jit_addi(j, v, R_FP, jit_allocai(j, sizeof(Value)));
			auto tmp = ralloc();
			jit_movi(j, tmp, 0);
			jit_stxi(j, offsetof(Value, a), v, tmp, sizeof(Value::a));
			jit_movi(j, tmp, VALUE_STRING);
			jit_stxi(j, offsetof(Value, type), v, tmp, sizeof(Value::type));
			jit_stxi(j, offsetof(Value, string), v, str, sizeof(Value::string));

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, v);
			jit_call_method(j, &Stack::push);
			
			rfree(str);
			rfree(v);
			rfree(tmp);
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

			auto str = ralloc();
			jit_ref_data(j, str, l);

			auto v = ralloc();
			jit_addi(j, v, R_FP, jit_allocai(j, sizeof(Value)));
			auto tmp = ralloc();
			jit_movi(j, tmp, 0);
			jit_stxi(j, offsetof(Value, a), v, tmp, sizeof(Value::a));
			jit_movi(j, tmp, VALUE_REFERENCE);
			jit_stxi(j, offsetof(Value, type), v, tmp, sizeof(Value::type));
			jit_stxi(j, offsetof(Value, string), v, str, sizeof(Value::string));

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, v);
			jit_call_method(j, &Stack::push);
			
			rfree(str);
			rfree(v);
			rfree(tmp);
			break;
		}
		default:
			assert(false);
			break;
	}
}