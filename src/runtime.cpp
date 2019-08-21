#include <string.h>
#include <stdio.h>

#include "runtime.h"

static Value NIL = Value(VALUE_NIL);

bool Value::is_truthy() {
	switch(type) {
		case VALUE_FALSE:	
		case VALUE_NIL:
			return false;
		case VALUE_NUMBER:	
			return number != 0;
		case VALUE_TRUE:	
		case VALUE_STRING:
		case VALUE_REFERENCE:
		case VALUE_OBJECT:	
		case VALUE_LAMBDA:
			return true;
	}

	assert(false);
}

bool Value::operator==(const Value &other) {
	assert(type >= VALUE_TRUE && type <= VALUE_NATIVE);
	assert(other.type >= VALUE_TRUE && other.type <= VALUE_NATIVE);

	if(type != other.type) return false;

	switch(type) {
		case VALUE_TRUE:
		case VALUE_FALSE:
		case VALUE_NIL:
			return true;
		case VALUE_NUMBER:
			return number == other.number;
			break;
		case VALUE_REFERENCE:
		case VALUE_STRING:
			return strcmp(string, other.string) == 0;
		case VALUE_OBJECT:
		case VALUE_LAMBDA:
			return a == other.a;
	}

	return false;
}

void Stack::push(Value *v) {
	if(v->a) heap->mark_root(v->a);
	data.push(*v);
}

Value Stack::pop() {
	auto x = data.pop();
	if(x.a) heap->unmark_root(x.a);
	return x;
}

void Stack::pop_into(Value *v) {
	auto x = data.pop();
	if(x.a) heap->unmark_root(x.a);
	*v = x;
}

Value* Stack::peek() {
	assert(data.count > 0);
	return &data.data[data.count - 1];
}

void Stack::set_top(Value v) {
	assert(data.count > 0);
	
	auto old = data.data[data.count - 1];
	if(old.a) heap->unmark_root(old.a); 
	data.data[data.count - 1] = v;
	if(v.a) heap->mark_root(v.a);
}

void Stack::swap() {
	assert(data.count > 1);
	auto x = data.data[data.count - 1];
	data.data[data.count - 1] = data.data[data.count - 2];
	data.data[data.count - 2] = x;
}

void Stack::rot() {
	// TODO fix this
	assert(data.count > 2);
	auto x = data.data[data.count - 1];
	auto y = data.data[data.count - 2];
	auto z = data.data[data.count - 3];
	data.data[data.count - 1] = z;
	data.data[data.count - 2] = x;
	data.data[data.count - 3] = y;
}

void Scope::set(char *key, Value *value) {
	for(Scope *curr = this; curr; curr = curr->parent) {
		if(curr->values.put_if_contains(key, value)) {
			return;
		}
	}
	values.put(key, *value);
}

Value* Scope::get(char *key) {
	Scope *curr = this;

	while(curr) {
		auto x = curr->values.get_ptr(key);
		if(x) return x;
		curr = curr->parent;
	}

	return &NIL;
}

void Scope::pop(Heap *heap) {
	if(!values.keys) return;
	if(values.count == 0) return;
	
	for(u32 i = 0; i < values.size; i++) {
		if(HT_HASH_IS_OCCUPIED(values.hashes[i])) {
			auto v = values.values[i];
			if(v.a) heap->unmark_root(v.a);
		}
	}
}

static u64 __value_hash(Value v) {
	switch(v.type) {
		case VALUE_TRUE:
			return 1;
		case VALUE_FALSE:
			return 0;
		case VALUE_NIL:
			return 2;
		case VALUE_NUMBER:
			return hash_u64(v.number);
		case VALUE_STRING:
		case VALUE_REFERENCE:
			return hash_string(v.string);
		case VALUE_OBJECT:
		case VALUE_LAMBDA:
			return hash_ptr(v.a);
	}

	assert(false);
	return 0;
}

static inline void call(Value v, Heap *heap, Scope *scope, Stack *stack) {
	assert(v.type == VALUE_LAMBDA || v.type == VALUE_NATIVE);

	auto s = Scope(scope);

	(*v.lambda.fn)(heap, &s, stack);

	s.pop(heap);
}

void __rt_eq(Stack *stack) {
	auto b = stack->pop();
	auto a = *stack->peek();

	Value c;
	c.type = a == b ? VALUE_TRUE : VALUE_FALSE;
	stack->set_top(c);
}

void __rt_lt(Stack *stack) {
	auto b = stack->pop();
	auto a = *stack->peek();

	Value c;

	if(a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) {
		c.type = a.number < b.number ? VALUE_TRUE : VALUE_FALSE;
	} else {
		assert(false);
	}

	stack->set_top(c);
}

void __rt_gt(Stack *stack) {
	auto b = stack->pop();
	auto a = *stack->peek();

	Value c;

	if(a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) {
		c.type = a.number > b.number ? VALUE_TRUE : VALUE_FALSE;
	} else {
		assert(false);
	}

	stack->set_top(c);
}

void __rt_cond_exec(Heap *heap, Scope *scope, Stack *stack) {
	auto body = stack->pop();
	auto cond = stack->pop();
	if(cond.is_truthy()) {
		call(body, heap, scope, stack);
	}
}

void __rt_exec(Heap *heap, Scope *scope, Stack *stack) {
	auto lambda = stack->pop();
	call(lambda, heap, scope, stack);
}

void __rt_and(Stack *stack) {
	auto b = stack->pop();
	auto a = *stack->peek();

	Value c;

	if((a.type == VALUE_TRUE || a.type == VALUE_FALSE) && (b.type == VALUE_TRUE || b.type == VALUE_FALSE)) {
		c.type = a.type == VALUE_TRUE && b.type == VALUE_TRUE ? VALUE_TRUE : VALUE_FALSE;
	} else if(a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) {
		c.type = VALUE_NUMBER;
		c.number = a.number & b.number;
	} else {
		assert(false);
	}

	stack->set_top(c);
}

void __rt_or(Stack *stack) {
	auto b = stack->pop();
	auto a = *stack->peek();

	Value c;

	if((a.type == VALUE_TRUE || a.type == VALUE_FALSE) && (b.type == VALUE_TRUE || b.type == VALUE_FALSE)) {
		c.type = a.type == VALUE_TRUE || b.type == VALUE_TRUE ? VALUE_TRUE : VALUE_FALSE;
	} else if(a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) {
		c.type = VALUE_NUMBER;
		c.number = a.number | b.number;
	} else {
		assert(false);
	}

	stack->set_top(c);
}

void __rt_not(Stack *stack) {
	auto a = stack->peek();

	if(a->type == VALUE_TRUE) {
		a->type = VALUE_FALSE;
	} else if(a->type == VALUE_FALSE || a->type == VALUE_NIL) {
		a->type = VALUE_TRUE;
	} else {
		assert(false);
	}
}

void __rt_add(Stack *stack) {
	auto b = stack->pop();
	auto a = *stack->peek();

	Value c;

	if(a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) {
		c.type = VALUE_NUMBER;
		c.number = a.number + b.number;
	} else if(a.type == VALUE_STRING || b.type == VALUE_STRING) {
		// TODO: string concatenation
		assert(false);
	}

	stack->set_top(c);
}

void __rt_sub(Stack *stack) {
	auto b = stack->pop();
	auto a = *stack->peek();

	Value c;

	if(a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) {
		c.type = VALUE_NUMBER;
		c.number = a.number - b.number;
	} else {
		assert(false);
	}

	stack->set_top(c);
}

void __rt_mul(Stack *stack) {
	auto b = stack->pop();
	auto a = *stack->peek();

	Value c;

	if(a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) {
		c.type = VALUE_NUMBER;
		c.number = a.number * b.number;
	} else {
		assert(false);
	}

	stack->set_top(c);
}

void __rt_div(Stack *stack) {
	auto b = stack->pop();
	auto a = *stack->peek();

	Value c;

	if(a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) {
		c.type = VALUE_NUMBER;
		c.number = a.number / b.number;
	} else {
		assert(false);
	}

	stack->set_top(c);
}

void __rt_neg(Stack *stack) {
	auto a = stack->peek();

	if(a->type == VALUE_NUMBER) {
		a->number = -a->number;
	} else {
		assert(false);
	}
}

void __rt_mod(Stack *stack) {
	auto b = stack->pop();
	auto a = *stack->peek();

	Value c;

	if(a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) {
		c.type = VALUE_NUMBER;
		c.number = a.number % b.number;
	} else {
		assert(false);
	}

	stack->set_top(c);
}

void __rt_newobj(Stack *stack, Heap *heap) {
	auto v = GC_ALLOC(heap);
	v->type = VALUE_OBJECT;
	v->object = (Hash_Table<Value, Value>*) ALLOC(cstdlib_allocator, sizeof(Hash_Table<Value, Value>));
	*v->object = Hash_Table<Value, Value>(__value_hash);
	stack->push(v);
}

void __rt_get_prop(Stack *stack) {
	auto key = stack->pop();
	auto object = stack->pop();

	assert(object.type == VALUE_OBJECT);

	auto x = object.object->get_ptr(key);
	if(x) {
		stack->push(x);
	} else {
		stack->push(&NIL);
	}
}

void __rt_set_prop(Stack *stack) {
	auto value = stack->pop();
	auto key = stack->pop();
	auto object = stack->pop();

	assert(object.type == VALUE_OBJECT);

	object.object->put(key, value);
}

void __rt_store(Heap *heap, Scope *scope, Stack *stack) {
	auto key = stack->pop();
	auto value = stack->pop();

	assert(key.type == VALUE_REFERENCE);

	auto old = scope->get(key.string);
	if(old && old->a) heap->unmark_root(old->a);
	if(value.a) heap->mark_root(value.a);

	scope->set(key.string, &value);
}

void __rt_load(Stack *stack, Scope *scope) {
	auto key = stack->pop();

	assert(key.type == VALUE_REFERENCE);

	stack->push(scope->get(key.string));
}

void __rt_while(Heap *heap, Scope *scope, Stack *stack) {
	auto body = stack->pop();
	auto cond = stack->pop();

	assert(body.type == VALUE_LAMBDA || body.type == VALUE_NATIVE);
	assert(cond.type == VALUE_LAMBDA || cond.type == VALUE_NATIVE);

	for(;;) {
		(*cond.lambda.fn)(heap, scope, stack);

		auto v = stack->pop();
		if(!v.is_truthy()) break;

		(*body.lambda.fn)(heap, scope, stack);
	}
}

void __std_print(Heap *heap, Scope *scope, Stack *stack) {
	// TOOD: tostring
	auto v = stack->pop();
	switch(v.type) {
		case VALUE_TRUE:
			printf("true");
			break;
		case VALUE_FALSE:
			printf("false");
			break;
		case VALUE_NIL:
			printf("nil");
			break;
		case VALUE_NUMBER:
			printf("%lli", v.number);
			break;
		case VALUE_STRING:
		case VALUE_REFERENCE:
			printf("%s", v.string);
			break;
		case VALUE_OBJECT:
			// TODO
			printf("{}");
			break;
		case VALUE_LAMBDA:
			printf("λ:%p", v.a);
			break;
	}
}

void __std_println(Heap *heap, Scope *scope, Stack *stack) {
	__std_print(heap, scope, stack);
	printf("\n");
}