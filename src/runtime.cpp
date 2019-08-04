#include "runtime.h"

u64 Value::hash() {
	return 0;
}

void Stack::push(Value *v) {
	data.add(v);
}

Value* Stack::pop() {
	assert(data.count - 1 >= 0);
	auto i = data.count - 1;
	auto v = data.data[i];
	assert(data.unordered_remove(i));
	return v;
}

Scope* Scope::push(u32 size) {
	auto s = new Scope();
	s->parent = this;
	s->values = Array<Value*>((u64)size);
	return s;
}

void Scope::pop() {

}