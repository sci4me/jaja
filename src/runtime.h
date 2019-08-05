#ifndef RUNTIME_H
#define RUNTIME_H

extern "C" {
#include "myjit/jitlib.h"
}

#include "hash_table.h"
#include "array.h"

#define VALUE_TRUE 0
#define VALUE_FALSE 1
#define VALUE_NIL 2
#define VALUE_NUMBER 3
#define VALUE_STRING 4
#define VALUE_REFERENCE 5
#define VALUE_OBJECT 6
#define VALUE_LAMBDA 7

struct Heap;
struct Allocation;
struct Scope;
struct Stack;

typedef void (* lambda_fn)(Heap*, Scope*, Stack*);

struct Lambda {
	jit *j;
	lambda_fn fn;
};

struct Value {
	Allocation *a;

	u8 type;
	union {
		s64 number;
		char *string;
		Hash_Table<Value, Value>* object;
		Lambda lambda;
	};

	Value() {}

	bool is_truthy();

	bool operator==(const Value &other);
};

struct Allocation {
	Allocation *next;
	bool marked;
	Value value;
};

struct Heap {
	Allocation *head;
	Array<Value> roots;

	Value alloc();
	void mark_root(Value v);
	void unmark_root(Value v);
	void gc();

	void mark(Value value);
	void sweep();
};

struct Stack {
	Array<Value> data;

	void push(Value v);
	Value pop();
};

struct Scope {
	Scope *parent;
	Hash_Table<char*, Value> values;

	Scope(Scope *_parent) : parent(_parent), values(Hash_Table<char*, Value>(hash_string, eq_string)) {}

	Scope *push();
	void pop(Heap *heap);

	void set(char *key, Value value);
	Value get(char *key);
	bool contains(char *key);
};

void __rt_eq(Stack *stack);
void __rt_lt(Stack *stack);
void __rt_gt(Stack *stack);

void __rt_cond_exec(Heap *heap, Scope *scope, Stack *stack);
void __rt_exec(Heap *heap, Scope *scope, Stack *stack);

void __rt_and(Stack *stack);
void __rt_or(Stack *stack);
void __rt_not(Stack *stack);

void __rt_add(Stack *stack);
void __rt_sub(Stack *stack);
void __rt_mul(Stack *stack);
void __rt_div(Stack *stack);
void __rt_neg(Stack *stack);
void __rt_mod(Stack *stack);

void __rt_newobj(Stack *stack, Heap *heap);
void __rt_get_prop(Stack *stack);
void __rt_set_prop(Stack *stack);

void __rt_dup(Stack *stack);
void __rt_drop(Stack *stack);
void __rt_swap(Stack *stack);
void __rt_rot(Stack *stack);

void __rt_load(Stack *stack, Scope *scope);
void __rt_store(Heap *heap, Scope *scope, Stack *stack);

void __rt_while(Heap *heap, Scope *scope, Stack *stack);

void __rt_push_true(Stack *stack);
void __rt_push_false(Stack *stack);
void __rt_push_nil(Stack *stack);
void __rt_push_number(Stack *stack, s64 n);
void __rt_push_string(Stack *stack, char *s);
void __rt_push_reference(Stack *stack, char *r);
void __rt_push_lambda(Stack *stack, Heap *heap, jit *j, lambda_fn fn);

void __rt_epilogue(Scope *scope, Heap *heap);

void __std_print(Heap *heap, Scope *scope, Stack *stack);
void __std_println(Heap *heap, Scope *scope, Stack *stack);

#endif