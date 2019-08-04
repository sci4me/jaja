#include "runtime.h"

u64 Value::hash() {
	return 0;
}

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