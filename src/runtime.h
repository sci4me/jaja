#ifndef RUNTIME_H
#define RUNTIME_H

extern "C" {
#include "myjit/jitlib.h"
}

#include "hash_table.h"
#include "array.h"

#define VALUE_TRUE 			0x00
#define VALUE_FALSE			0x01
#define VALUE_NIL 			0x02
#define VALUE_NUMBER 		0x03
#define VALUE_STRING 		0x04
#define VALUE_REFERENCE 	0x05
#define VALUE_OBJECT 		0x06
#define VALUE_LAMBDA 		0x07
#define VALUE_NATIVE 		0x08

#define HEAP_DEBUG

#ifdef HEAP_DEBUG
	#define GC_ALLOC(h) h->alloc(__LINE__, __func__, __FILE__)
#else
	#define GC_ALLOC(h) h->alloc();
#endif

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

	Value() : a(0) {}

	bool is_truthy();

	bool operator==(const Value &other);
};

struct Allocation {
	bool marked;
	Value value;

#ifdef HEAP_DEBUG
	u32 line;
	const char *func;
	const char *file;
#endif
};

struct Heap {
	Allocator allocator;

	Array<Allocation*> allocations;
	Array<Allocation*> roots;

	Heap(Allocator _allocator = cstdlib_allocator) : allocator(_allocator) {}
	~Heap();

#ifdef HEAP_DEBUG
	Value* alloc(u32 line, const char *func, const char *file);
#else
	Value* alloc();
#endif
	void mark_root(Allocation *a);
	void unmark_root(Allocation *a);
	void gc();

	u32 mark(Allocation *a);
	u32 sweep();
};

struct Stack {
	Heap *heap;
	Array<Value> data;

	Stack(Heap *_heap) : heap(_heap) { }

	void push(Value v);
	Value pop();
	Value peek();
	void set_top(Value v);
};

struct Scope {
	Scope *parent;
	Hash_Table<char*, Value> values;
	bool popped;

	Scope(Scope *_parent) : parent(_parent), values(Hash_Table<char*, Value>(hash_string, eq_string)), popped(false) {}

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