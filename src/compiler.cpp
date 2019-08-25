#include "compiler.h"

#include <jit/jit-dump.h>

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

Compiler::Compiler() : labels(Hash_Table<u64, jit_label_t>(hash_u64)) {
	ctx = jit_context_create();

	{
		jit_type_t params[] = { jit_type_void_ptr };
		signature_void_void_ptr_1 = jit_type_create_signature(jit_abi_cdecl, jit_type_void, params, 1, 0);
	}

	{
		jit_type_t params[] = { jit_type_void_ptr, jit_type_void_ptr };
		signature_void_void_ptr_2 = jit_type_create_signature(jit_abi_cdecl, jit_type_void, params, 2, 0);
	}

	{
		jit_type_t params[] = { jit_type_void_ptr, jit_type_void_ptr, jit_type_void_ptr };
		signature_void_void_ptr_3 = jit_type_create_signature(jit_abi_cdecl, jit_type_void, params, 3, 0);
	}

	{
		jit_type_t params[] = { jit_type_void_ptr };
		signature_ubyte_void_ptr_1 = jit_type_create_signature(jit_abi_cdecl, jit_type_ubyte, params, 1, 0);
	}
}

Compiler::~Compiler() {
	jit_context_destroy(ctx);

	jit_type_free(signature_void_void_ptr_1);
	jit_type_free(signature_void_void_ptr_2);
	jit_type_free(signature_void_void_ptr_3);
	jit_type_free(signature_ubyte_void_ptr_1);
}

void Compiler::start() {
	jit_context_build_start(ctx);
}

void Compiler::end() {
	jit_context_build_end(ctx);
}

Value Compiler::compile(Array<Node*> *ast) {
	Value result;
	result.a = NULL;
	result.type = VALUE_LAMBDA;
	result.lambda = compile_raw(ast);
	return result;
}

Lambda Compiler::compile_raw(Array<Node*> *ast) {
	Lambda result;

	auto j = jit_function_create(ctx, signature_void_void_ptr_3);
	result.j = j;

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

	jit_insn_return(j, 0);

	jit_dump_function(stdout, j, 0);
	jit_function_compile(j);
	// jit_dump_function(stdout, j, 0);

	return result;
}

void Compiler::compile_lambda(jit_function_t j, Node *n) {
	auto l = compile(&n->lambda);

	auto value_size = jit_value_create_nint_constant(j, jit_type_int, sizeof(Value));
	auto v = jit_insn_alloca(j, value_size);
	auto zero = jit_value_create_nint_constant(j, jit_type_ubyte, 0);
	jit_insn_memset(j, v, zero, value_size);

	auto type = jit_value_create_nint_constant(j, jit_type_ubyte, VALUE_LAMBDA);
	jit_insn_store_relative(j, v, offsetof(Value, type), type);
	auto _j = jit_value_create_long_constant(j, jit_type_void_ptr, (jit_long) l.lambda.j);
	jit_insn_store_relative(j, v, offsetof(Value, lambda.j), _j);

	auto stack = jit_value_get_param(j, 2);
	jit_value_t args[] = { stack, v };
	jit_insn_call_native_method(j, "Stack::push", &Stack::push, signature_void_void_ptr_2, args, 2, JIT_CALL_NOTHROW);
}

#define XSTR(x) #x
#define STR(x) XSTR(x)

#define JIT_CALL_2(name) { auto stack = jit_value_get_param(j, 2); jit_value_t args[] = { stack }; jit_insn_call_native(j, STR(name), (void*)name, signature_void_void_ptr_1, args, 1, JIT_CALL_NOTHROW); }
#define JIT_CALL_20(name) { auto stack = jit_value_get_param(j, 2); auto heap = jit_value_get_param(j, 0); jit_value_t args[] = { stack, heap }; jit_insn_call_native(j, STR(name), (void*)name, signature_void_void_ptr_2, args, 2, JIT_CALL_NOTHROW); }
#define JIT_CALL_21(name) { auto stack = jit_value_get_param(j, 2); auto scope = jit_value_get_param(j, 1); jit_value_t args[] = { stack, scope }; jit_insn_call_native(j, STR(name), (void*)name, signature_void_void_ptr_2, args, 2, JIT_CALL_NOTHROW); }
#define JIT_CALL_012(name) { auto heap = jit_value_get_param(j, 0); auto scope = jit_value_get_param(j, 1); auto stack = jit_value_get_param(j, 2); jit_value_t args[] = { heap, scope, stack }; jit_insn_call_native(j, STR(name), (void*)name, signature_void_void_ptr_3, args, 3, JIT_CALL_NOTHROW); }

void Compiler::compile_instruction(jit_function_t j, Node *n, Array<Node*> *ast, u32 index) {
	switch(n->op) {
		case AST_OP_EQ:
			JIT_CALL_2(__rt_eq);
			break;
		case AST_OP_LT:
			JIT_CALL_2(__rt_lt);
			break;
		case AST_OP_GT:
			JIT_CALL_2(__rt_gt);
			break;
		case AST_OP_COND_EXEC:
			JIT_CALL_012(__rt_cond_exec);
			break;
		case AST_OP_EXEC:
			JIT_CALL_012(__rt_exec);
			break;
		case AST_OP_AND:
			JIT_CALL_2(__rt_and);
			break;
		case AST_OP_OR:
			JIT_CALL_2(__rt_or);
			break;
		case AST_OP_NOT:
			JIT_CALL_2(__rt_not);
			break;
		case AST_OP_ADD:
			JIT_CALL_2(__rt_add);
			break;
		case AST_OP_SUB:
			JIT_CALL_2(__rt_sub);
			break;
		case AST_OP_MUL:
			JIT_CALL_2(__rt_mul);
			break;
		case AST_OP_DIV:
			JIT_CALL_2(__rt_div);
			break;
		case AST_OP_NEG:
			JIT_CALL_2(__rt_neg);
			break;
		case AST_OP_MOD:
			JIT_CALL_2(__rt_mod);
			break;
		case AST_OP_NEW_OBJECT:
			JIT_CALL_20(__rt_newobj);
			break;
		case AST_OP_GET_PROP:
			JIT_CALL_2(__rt_get_prop);
			break;
		case AST_OP_SET_PROP:
			JIT_CALL_2(__rt_set_prop);
			break;
		case AST_OP_DUP: {
			auto stack = jit_value_get_param(j, 2);
			jit_value_t args[] = { stack };
			jit_insn_call_native_method(j, "Stack::dup", &Stack::dup, signature_void_void_ptr_1, args, 1, JIT_CALL_NOTHROW);
			break;
		}
		case AST_OP_DROP: {
			auto stack = jit_value_get_param(j, 2);
			jit_value_t args[] = { stack };
			jit_insn_call_native_method(j, "Stack::drop", &Stack::drop, signature_void_void_ptr_1, args, 1, JIT_CALL_NOTHROW);
			break;
		}
		case AST_OP_SWAP: {
			auto stack = jit_value_get_param(j, 2);
			jit_value_t args[] = { stack };
			jit_insn_call_native_method(j, "Stack::swap", &Stack::swap, signature_void_void_ptr_1, args, 1, JIT_CALL_NOTHROW);
			break;
		}
		case AST_OP_ROT: {
			auto stack = jit_value_get_param(j, 2);
			jit_value_t args[] = { stack };
			jit_insn_call_native_method(j, "Stack::rot", &Stack::rot, signature_void_void_ptr_1, args, 1, JIT_CALL_NOTHROW);
			break;
		}
		case AST_OP_LOAD:
			JIT_CALL_21(__rt_load);
			break;
		case AST_OP_STORE:
			JIT_CALL_012(__rt_store);
			break;
		case AST_OP_WHILE:
			JIT_CALL_012(__rt_while);
			break;
		case AST_OP_BRANCH: {
			auto label = labels.get_or_else(n->label, jit_label_undefined);
			jit_insn_branch(j, &label);
			labels.put(n->label, label);
			break;
		}
		case AST_OP_BRANCH_IF_FALSE:
		case AST_OP_BRANCH_IF_TRUE: {
			auto value_size = jit_value_create_nint_constant(j, jit_type_long, sizeof(Value));
			auto v = jit_insn_alloca(j, value_size);

			auto stack = jit_value_get_param(j, 2);

			jit_value_t pop_into_args[] = { stack, v };
			jit_insn_call_native_method(j, "Stack::pop_into", &Stack::pop_into, signature_void_void_ptr_2, pop_into_args, 2, JIT_CALL_NOTHROW);

			jit_value_t is_truthy_args[] = { v };			
			auto fptr = &Value::is_truthy;
			auto result = jit_insn_call_native(j, "Value::is_truthy", reinterpret_cast<void*&>(fptr), signature_ubyte_void_ptr_1, is_truthy_args, 1, JIT_CALL_NOTHROW);

			auto label = labels.get_or_else(n->label, jit_label_undefined);
		
			auto zero = jit_value_create_nint_constant(j, jit_type_ubyte, 0);
			jit_value_t cond;
			if(n->op == AST_OP_BRANCH_IF_FALSE) {
				cond = jit_insn_eq(j, result, zero);
			} else {
				cond = jit_insn_ne(j, result, zero);
			}
			jit_insn_branch_if(j, cond, &label);

			labels.put(n->label, label);
			break;
		}
		case AST_OP_BRANCH_TARGET: {
			auto label = labels.get_or_else(n->label, jit_label_undefined);
			jit_insn_label(j, &label);
			labels.put(n->label, label);
			break;
		}
		default:
			assert(false);
	}
}

void Compiler::compile_constant(jit_function_t j, Node *n) {
	auto value_size = jit_value_create_nint_constant(j, jit_type_int, sizeof(Value));
	auto v = jit_insn_alloca(j, value_size);
	auto zero = jit_value_create_nint_constant(j, jit_type_ubyte, 0);
	jit_insn_memset(j, v, zero, value_size);

	switch(n->type) {
		case NODE_TRUE: {
			auto type = jit_value_create_nint_constant(j, jit_type_ubyte, VALUE_TRUE);
			jit_insn_store_relative(j, v, offsetof(Value, type), type);
			break;
		}
		case NODE_FALSE: {
			auto type = jit_value_create_nint_constant(j, jit_type_ubyte, VALUE_FALSE);
			jit_insn_store_relative(j, v, offsetof(Value, type), type);
			break;
		}
		case NODE_NIL: {
			auto type = jit_value_create_nint_constant(j, jit_type_ubyte, VALUE_NIL);
			jit_insn_store_relative(j, v, offsetof(Value, type), type);
			break;
		}
		case NODE_NUMBER: {
			auto type = jit_value_create_nint_constant(j, jit_type_ubyte, VALUE_NUMBER);
			jit_insn_store_relative(j, v, offsetof(Value, type), type);
			auto number = jit_value_create_nint_constant(j, jit_type_long, n->number);
			jit_insn_store_relative(j, v, offsetof(Value, number), number);
			break;
		}
		case NODE_STRING: {
			auto type = jit_value_create_nint_constant(j, jit_type_ubyte, VALUE_STRING);
			jit_insn_store_relative(j, v, offsetof(Value, type), type);
			auto str = jit_value_create_long_constant(j, jit_type_void_ptr, (jit_long) n->string);
			jit_insn_store_relative(j, v, offsetof(Value, string), str);
			break;
		}
		case NODE_REFERENCE: {
			auto type = jit_value_create_nint_constant(j, jit_type_ubyte, VALUE_REFERENCE);
			jit_insn_store_relative(j, v, offsetof(Value, type), type);
			auto str = jit_value_create_long_constant(j, jit_type_void_ptr, (jit_long) n->string);
			jit_insn_store_relative(j, v, offsetof(Value, string), str);
			break;
		}
		default:
			assert(false);
			break;
	}

	auto stack = jit_value_get_param(j, 2);
	jit_value_t args[] = { stack, v };
	jit_insn_call_native_method(j, "Stack::push", &Stack::push, signature_void_void_ptr_2, args, 2, JIT_CALL_NOTHROW);
}