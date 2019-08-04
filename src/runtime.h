#ifndef RUNTIME_H
#define RUNTIME_H

extern "C" {
#include "myjit/jitlib.h"
}

#include "hash_table.h"
#include "array.h"

#define VALUE_TRUE 0
#define VALUE_FALSE 1
#define VALUE_NUMBER 2
#define VALUE_STRING 3
#define VALUE_REFERENCE 4
#define VALUE_OBJECT 5
#define VALUE_LAMBDA 6

struct Allocation;

struct Stack;
struct Scope;

typedef void (* lambda_fn)(Scope*, Stack*);

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
		Lambda *lambda;
	};

	Value() {}

	u64 hash();
};

struct Stack {
	Array<Value*> data;

	void push(Value *v);
	Value* pop();
};

struct Scope {
	Scope *parent;
	Array<Value*> values;

	Scope *push(u32 size);
	void pop();

	void set(u32 key, Value *value);
	Value* get(u32 key);
};

#endif