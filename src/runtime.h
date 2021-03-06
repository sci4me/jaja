#ifndef RUNTIME_H
#define RUNTIME_H

#include <jit/jit.h>

#include "hash_table.h"
#include "array.h"

// #define HEAP_DEBUG

#define VALUE_TRUE 			0x00
#define VALUE_FALSE			0x01
#define VALUE_NIL 			0x02
#define VALUE_NUMBER 		0x03
#define VALUE_STRING 		0x04
#define VALUE_REFERENCE 	0x05
#define VALUE_OBJECT 		0x06
#define VALUE_LAMBDA 		0x07
#define VALUE_NATIVE 		0x08

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
	jit_function_t j;
};

struct Value {
	Allocation *a;

	u8 type;
	union {
		s64 number;
		char *string;
		Hash_Table<Value, Value> *object;
		lambda_fn native;
		Lambda lambda;
	};

	Value() : a(0) {}
	Value(u8 _type) : a(0), type(_type) {}

	bool is_truthy();

	bool operator==(const Value &other);
};

struct Allocation {
	Allocation *next;
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

	Allocation *head;
	u32 allocations;
	Array<Allocation*> roots;

	Heap(Allocator _allocator = cstdlib_allocator) : allocator(_allocator), allocations(0), head(NULL) {}

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

	void push(Value *v);
	Value pop();
	void pop_into(Value *v);
	Value* peek();
	void set_top(Value v);

	void dup();
	void drop();
	void swap();
	void rot();
};

struct Scope {
	Heap *heap;
	Scope *parent;
	Hash_Table<char*, Value> values;

	Scope(Heap *_heap) : parent(NULL), heap(_heap), values(Hash_Table<char*, Value>(hash_string, eq_string)) {}
	Scope(Scope *_parent) : parent(_parent), heap(_parent->heap), values(Hash_Table<char*, Value>(hash_string, eq_string)) {}

	Scope *push();
	void pop(Heap *heap);

	void set(char *key, Value *value);
	Value* get(char *key);
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

void __rt_load(Stack *stack, Scope *scope);
void __rt_store(Heap *heap, Scope *scope, Stack *stack);

void __rt_while(Heap *heap, Scope *scope, Stack *stack);

void __std_print(Heap *heap, Scope *scope, Stack *stack);
void __std_println(Heap *heap, Scope *scope, Stack *stack);

#endif