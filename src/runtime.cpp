#include "runtime.h"

bool Value::operator==(const Value &b) {
	// TODO ?
	return a == b.a;
}

void Stack::push(Value v) {
	data.add(v);
}

Value Stack::pop() {
	assert(data.count - 1 >= 0);
	auto i = data.count - 1;
	auto v = data.data[i];
	assert(data.unordered_remove(i));
	return v;
}

Scope* Scope::push(u32 size) {
	auto s = new Scope();
	s->parent = this;
	s->values = Array<Value>((u64)size);
	return s;
}

void Scope::pop() {
	// TODO
}

static u64 __value_hash(Value v) {
	return 0;
}

void __rt_eq(Stack *stack) {
	// TODO
}

void __rt_lt(Stack *stack) {
	// TODO
}

void __rt_gt(Stack *stack) {
	// TODO
}

void __rt_cond_exec(Stack *stack) {
	// TODO
}

void __rt_exec(Stack *stack) {
	// TODO
}

void __rt_and(Stack *stack) {
	// TODO
}

void __rt_or(Stack *stack) {
	// TODO
}

void __rt_not(Stack *stack) {
	// TODO
}

void __rt_add(Stack *stack) {
	// TODO
}

void __rt_sub(Stack *stack) {
	// TODO
}

void __rt_mul(Stack *stack) {
	// TODO
}

void __rt_div(Stack *stack) {
	// TODO
}

void __rt_neg(Stack *stack) {
	// TODO
}

void __rt_mod(Stack *stack) {
	// TODO
}

void __rt_newobj(Stack *stack, Heap *heap) {
	auto v = heap->alloc();
	v.type = VALUE_OBJECT;
	v.object = new Hash_Table<Value, Value>(__value_hash);
	stack->push(v);
}

void __rt_get_prop(Stack *stack) {
	// TODO
}

void __rt_set_prop(Stack *stack) {
	// TODO
}

void __rt_dup(Stack *stack) {
	// TODO
}

void __rt_drop(Stack *stack) {
	// TODO
}

void __rt_swap(Stack *stack) {
	// TODO
}

void __rt_rot(Stack *stack) {
	// TODO
}

void __rt_load(Stack *stack, Scope *scope) {
	// TODO
}

void __rt_store(Stack *stack, Scope *scope) {
	// TODO
}

void __rt_while(Stack *stack) {
	// TODO
}

void __rt_push_true(Stack *stack) {
	Value v;
	v.type = VALUE_TRUE;
	stack->push(v);
}

void __rt_push_false(Stack *stack) {
	Value v;
	v.type = VALUE_FALSE;
	stack->push(v);
}

void __rt_push_nil(Stack *stack) {
	Value v;
	v.type = VALUE_NIL;
	stack->push(v);
}

void __rt_push_number(Stack *stack, s64 n) {
	Value v;
	v.type = VALUE_NUMBER;
	v.number = n;
	stack->push(v);
}

void __rt_push_string(Stack *stack, char *s) {
	Value v;
	v.type = VALUE_STRING;
	v.string = s;
	stack->push(v);
}

void __rt_push_reference(Stack *stack, u64 r) {
	Value v;
	v.type = VALUE_REFERENCE;
	v.number = r;
	stack->push(v);
}