#include "compiler.h"

/*

It seems to be the case that changing the JIT to generate code that does more of the work itself rather than calling out to a runtime function
is causing the JIT to run MUCH more slowly, possibly exponentially more slowly. Why? We _REALLY_ need to figure this out. Even if we do end up
having an interpreter as a first stage for run-once code, before the JIT, uh.... yeahhhh.... uh...
We can't take hundreds of millis to compile a damn function. So. What's going on? Either I'm doing something dumb or myjit sucks or like...
Idk. #FigureItOut @FutureSelf
					- sci4me, Aug 17 2019

*/

#ifdef JIT_RALLOC_TRACKING
jit_value Compiler::ralloc(const char *func, const char *file, u32 line) {
#else
jit_value Compiler::ralloc() {
#endif
	u64 n = registers->next_clear();
	registers->set(n);
#ifdef JIT_RALLOC_TRACKING
	allocations->put(n, {
		.func = func,
		.file = file,
		.line = line
	});
#endif
#ifdef JIT_MAX_RALLOC_TRACKING
	if(n > max_rallocs) {
		max_rallocs = n;
	}
#endif
	return R(n);
}

void Compiler::rfree(jit_value r) {
	jit_reg x;
	memcpy(&x, &r, sizeof(jit_reg));
	auto n = (u64) x.id;

	assert(registers->get(n));
	registers->clear(n);

#ifdef JIT_RALLOC_TRACKING
	allocations->remove(n);
#endif
}

Value Compiler::compile(Array<Node*>* ast) {
	Value result;
	result.a = NULL;
	result.type = VALUE_LAMBDA;
	result.lambda = compile_raw(ast);
	return result;
}

Lambda Compiler::compile_raw(Array<Node*>* ast) {	
	registersStack.push(registers);
	registers = new Bitset();

#ifdef JIT_RALLOC_TRACKING
	allocationsStack.push(allocations);
	allocations = new Hash_Table<u64, RAllocation>(hash_u64);
#endif
#ifdef JIT_MAX_RALLOC_TRACKING
	max_rallocs_stack.push(max_rallocs);
	max_rallocs = 0;
#endif

	registers->set(0);
	registers->set(1);
	registers->set(2);

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

	registers->clear(0);
	registers->clear(1);
	registers->clear(2);

#ifdef JIT_RALLOC_TRACKING
	if(registers->bits_set()) {
		for(u32 i = 0; i < allocations->size; i++) {
			if(allocations->state[i] == HT_STATE_OCCUPIED) {
				auto a = allocations->values[i];
				printf("ralloc (%u) : %s@%s:%u\n", allocations->keys[i], a.func, a.file, a.line);
			}
		}
		fflush(stdout);
		assert(false);
	}
#endif
#ifdef JIT_MAX_RALLOC_TRACKING
	printf("max_rallocs: %u\n", max_rallocs);
	max_rallocs = max_rallocs_stack.pop();
#endif

	delete registers;
	registers = registersStack.pop();

#ifdef JIT_RALLOC_TRACKING
	delete allocations;
	allocations = allocationsStack.pop();
#endif

	return result;
}

void Compiler::compile_lambda(jit *j, Node *n) {
#ifdef JIT_DEBUG
	jit_comment(j, "__rt_push_lambda");
#endif

	auto l = compile(&n->lambda); // TODO: we need to free this memory (from jit_init) somehow

	auto lambda = RALLOC();
	jit_addi(j, lambda, R_FP, jit_allocai(j, sizeof(Value)));

	auto tmp = RALLOC();
	jit_movi(j, tmp, 0);
	jit_stxi(j, offsetof(Value, a), lambda, tmp, sizeof(Value::a));
	jit_movi(j, tmp, VALUE_LAMBDA);
	jit_stxi(j, offsetof(Value, type), lambda, tmp, sizeof(Value::type));
	jit_movi(j, tmp, l.lambda.j);
	jit_stxi(j, offsetof(Value, lambda.j), lambda, tmp, sizeof(Value::lambda.j));
	jit_movi(j, tmp, l.lambda.fn);
	jit_stxi(j, offsetof(Value, lambda.fn), lambda, tmp, sizeof(Value::lambda.fn));

	jit_prepare(j);
	jit_putargr(j, R_STACK);
	jit_putargr(j, lambda);
	jit_call_method(j, &Stack::push);
	
	RFREE(lambda);
	RFREE(tmp);

/*
#ifdef HEAP_DEBUG
	auto lambda = RALLOC(); auto LINE = __LINE__;
	auto rz = RALLOC();

	jit_movi(j, rz, 0);
	jit_prepare(j);
	jit_putargr(j, R_HEAP);
	jit_putargi(j, LINE);
	jit_putargi(j, __func__);
	jit_putargi(j, __FILE__);
	jit_call_method(j, &Heap::alloc);
	jit_retval(j, lambda);

	RFREE(rz);
#else
	auto lambda = RALLOC();

	jit_prepare(j);
	jit_putargr(j, R_HEAP);
	jit_call_method(j, &Heap::alloc);
	jit_retval(j, lambda);
#endif

	auto tmp = RALLOC();
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

	RFREE(lambda);
	RFREE(tmp);
*/
}

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
		case AST_OP_NOT: {
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_not");
#endif
			auto x = RALLOC();
			auto type = RALLOC();
			auto tmp = RALLOC();
		
			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_call_method(j, &Stack::peek);
			jit_retval(j, x);

			jit_ldxi(j, type, x, offsetof(Value, type), sizeof(Value::type));

			auto l1 = jit_bnei(j, 0, type, VALUE_TRUE);
				jit_movi(j, tmp, VALUE_FALSE);
				jit_stxi(j, offsetof(Value, type), x, tmp, sizeof(Value::type));
			auto j1 = jit_jmpi(j, JIT_FORWARD);
			jit_patch(j, l1);
			auto l2 = jit_bnei(j, 0, type, VALUE_FALSE);
				jit_movi(j, tmp, VALUE_TRUE);
				jit_stxi(j, offsetof(Value, type), x, tmp, sizeof(Value::type));
			auto j2 = jit_jmpi(j, JIT_FORWARD);
			jit_patch(j, l2);
#ifndef NDEBUG
			jit_prepare(j);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_call(j, __assert_fail);
#endif
			jit_patch(j, j1);
			jit_patch(j, j2);

			RFREE(x);
			RFREE(type);
			RFREE(tmp);
			break;
		}
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
		case AST_OP_SET_PROP: {
			JIT_RT_CALL_2(__rt_set_prop);
			/*
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_set_prop");
#endif

			auto value = RALLOC();
			auto key = RALLOC();
			auto object = RALLOC();

			jit_addi(j, value, R_FP, jit_allocai(j, sizeof(Value)));
			jit_addi(j, key, R_FP, jit_allocai(j, sizeof(Value)));
			jit_addi(j, object, R_FP, jit_allocai(j, sizeof(Value)));

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, value);
			jit_call_method(j, &Stack::pop_into);

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, key);
			jit_call_method(j, &Stack::pop_into);
			
			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, object);
			jit_call_method(j, &Stack::pop_into);

#ifndef NDEBUG
			auto type = RALLOC();
			jit_ldxi(j, type, object, offsetof(Value, type), sizeof(Value::type));

			auto l = jit_beqi(j, 0, type, VALUE_OBJECT);
			jit_prepare(j);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_call(j, __assert_fail);
			jit_patch(j, l);
			
			RFREE(type);
#endif

			auto _object = RALLOC();
			jit_ldxi(j, _object, object, offsetof(Value, object), sizeof(Value::object));

			jit_prepare(j);
			jit_putargr(j, _object);
			jit_putargr(j, key);			
			jit_putargr(j, value);
			auto x = &Hash_Table<Value, Value>::put_by_ptr;
			jit_call_method(j, x);	

			RFREE(value);
			RFREE(key);
			RFREE(object);
			RFREE(_object);
			*/		
			break;
		}
		case AST_OP_DUP: {
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_dup");
#endif
			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_call_method(j, &Stack::peek);
			auto tos = RALLOC();
			jit_retval(j, tos);

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, tos);
			jit_call_method(j, &Stack::push);
			RFREE(tos);
			break;
		}
		case AST_OP_DROP: {
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_drop");
#endif
			auto data = RALLOC();
			jit_addi(j, data, R_STACK, offsetof(Stack, data));
			jit_prepare(j);
			jit_putargr(j, data);
			jit_call_method(j, &Array<Value>::drop);
			RFREE(data);
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
			JIT_RT_CALL_21(__rt_load);
			/*
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_load");
#endif
			auto key = RALLOC();
			jit_addi(j, key, R_FP, jit_allocai(j, sizeof(Value)));

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, key);
			jit_call_method(j, &Stack::pop_into);

#ifndef NDEBUG
			auto type = RALLOC();
			jit_ldxi(j, type, key, offsetof(Value, type), sizeof(Value::type));

			auto l = jit_beqi(j, 0, type, VALUE_REFERENCE);
			jit_prepare(j);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_call(j, __assert_fail);
			jit_patch(j, l);
			RFREE(type);
#endif

			auto string = RALLOC();
			jit_ldxi(j, string, key, offsetof(Value, string), sizeof(Value::string));

			jit_prepare(j);
			jit_putargr(j, R_SCOPE);
			jit_putargr(j, string);
			jit_call_method(j, &Scope::get); // @Volatile
			auto result = RALLOC();
			jit_retval(j, result);

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, result);
			jit_call_method(j, &Stack::push);

			RFREE(key);
			RFREE(string);
			RFREE(result);
			*/
			break;
		}
		case AST_OP_STORE: {
			JIT_RT_CALL_012(__rt_store);
			/*
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_store");
#endif

			auto key = RALLOC();
			auto value = RALLOC();
			jit_addi(j, key, R_FP, jit_allocai(j, sizeof(Value)));
			jit_addi(j, value, R_FP, jit_allocai(j, sizeof(Value)));

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, key);
			jit_call_method(j, &Stack::pop_into);

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, value);
			jit_call_method(j, &Stack::pop_into);			

#ifndef NDEBUG
			auto type = RALLOC();
			jit_ldxi(j, type, key, offsetof(Value, type), sizeof(Value::type));

			auto l = jit_beqi(j, 0, type, VALUE_REFERENCE);
			jit_prepare(j);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_putargi(j, 0);
			jit_call(j, __assert_fail);
			jit_patch(j, l);
			
			RFREE(type);
#endif	

			auto string = RALLOC();
			jit_ldxi(j, string, key, offsetof(Value, string), sizeof(Value::string));

			jit_prepare(j);
			jit_putargr(j, R_SCOPE);
			jit_putargr(j, string);
			jit_putargr(j, value);
			jit_call_method(j, &Scope::set);

			RFREE(key);
			RFREE(value);
			RFREE(string);
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
			auto v = RALLOC();
			jit_addi(j, v, R_FP, jit_allocai(j, sizeof(Value)));
			auto tmp = RALLOC();
			jit_movi(j, tmp, 0);
			jit_stxi(j, offsetof(Value, a), v, tmp, sizeof(Value::a));
			jit_movi(j, tmp, VALUE_TRUE);
			jit_stxi(j, offsetof(Value, type), v, tmp, sizeof(Value::type));

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, v);
			jit_call_method(j, &Stack::push);

			RFREE(v);
			RFREE(tmp);
			break;
		}
		case NODE_FALSE: {
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_push_false");
#endif
			auto i = jit_allocai(j, sizeof(Value));

			auto v = RALLOC();
			jit_addi(j, v, R_FP, i);
			auto tmp = RALLOC();
			jit_movi(j, tmp, 0);
			jit_stxi(j, offsetof(Value, a), v, tmp, sizeof(Value::a));
			jit_movi(j, tmp, VALUE_FALSE);
			jit_stxi(j, offsetof(Value, type), v, tmp, sizeof(Value::type));

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, v);
			jit_call_method(j, &Stack::push);

			RFREE(v);
			RFREE(tmp);
			break;
		}
		case NODE_NIL: {
			#ifdef JIT_DEBUG
			jit_comment(j, "__rt_push_nil");
#endif
			auto i = jit_allocai(j, sizeof(Value));

			auto v = RALLOC();
			jit_addi(j, v, R_FP, i);
			auto tmp = RALLOC();
			jit_movi(j, tmp, 0);
			jit_stxi(j, offsetof(Value, a), v, tmp, sizeof(Value::a));
			jit_movi(j, tmp, VALUE_NIL);
			jit_stxi(j, offsetof(Value, type), v, tmp, sizeof(Value::type));

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, v);
			jit_call_method(j, &Stack::push);

			RFREE(v);
			RFREE(tmp);
			break;
		}
		case NODE_NUMBER: {
#ifdef JIT_DEBUG
			jit_comment(j, "__rt_push_number");
#endif
			auto v = RALLOC();
			jit_addi(j, v, R_FP, jit_allocai(j, sizeof(Value)));
			auto tmp = RALLOC();
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

			RFREE(v);
			RFREE(tmp);
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

			auto str = RALLOC();
			jit_ref_data(j, str, l);

			auto v = RALLOC();
			jit_addi(j, v, R_FP, jit_allocai(j, sizeof(Value)));
			auto tmp = RALLOC();
			jit_movi(j, tmp, 0);
			jit_stxi(j, offsetof(Value, a), v, tmp, sizeof(Value::a));
			jit_movi(j, tmp, VALUE_STRING);
			jit_stxi(j, offsetof(Value, type), v, tmp, sizeof(Value::type));
			jit_stxi(j, offsetof(Value, string), v, str, sizeof(Value::string));

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, v);
			jit_call_method(j, &Stack::push);
			
			RFREE(str);
			RFREE(v);
			RFREE(tmp);
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

			auto str = RALLOC();
			jit_ref_data(j, str, l);

			auto v = RALLOC();
			jit_addi(j, v, R_FP, jit_allocai(j, sizeof(Value)));
			auto tmp = RALLOC();
			jit_movi(j, tmp, 0);
			jit_stxi(j, offsetof(Value, a), v, tmp, sizeof(Value::a));
			jit_movi(j, tmp, VALUE_REFERENCE);
			jit_stxi(j, offsetof(Value, type), v, tmp, sizeof(Value::type));
			jit_stxi(j, offsetof(Value, string), v, str, sizeof(Value::string));

			jit_prepare(j);
			jit_putargr(j, R_STACK);
			jit_putargr(j, v);
			jit_call_method(j, &Stack::push);
			
			RFREE(str);
			RFREE(v);
			RFREE(tmp);
			break;
		}
		default:
			assert(false);
			break;
	}
}