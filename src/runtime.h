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
		Hash_Table<Value*, Value*>* object;
		Lambda lambda;
	};

	Value() {}

	u64 hash();

	bool operator==(const Value &b);
};

struct Stack {
	Array<Value> data;

	void push(Value v);
	Value pop();
};

struct Scope {
	Scope *parent;
	Array<Value> values;

	Scope *push(u32 size);
	void pop();

	void set(u32 key, Value value);
	Value get(u32 key);
};

void __rt_push_true(Stack *stack);
void __rt_push_false(Stack *stack);
void __rt_push_nil(Stack *stack);
void __rt_push_number(Stack *stack, s64 n);
void __rt_push_string(Stack *stack, char *s);
void __rt_push_reference(Stack *stack, u64 r);

#endif