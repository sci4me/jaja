#include <string.h>

#include "runtime.h"

bool Value::operator==(const Value &other) {
	if(type != other.type) return false;

	switch(type) {
		case VALUE_TRUE:
		case VALUE_FALSE:
		case VALUE_NIL:
			return true;
		case VALUE_NUMBER:
		case VALUE_REFERENCE:
			return number == other.number;
			break;
		case VALUE_STRING:
			return strcmp(string, other.string) == 0;
		case VALUE_OBJECT:
			assert(false); // TODO
			break;
		case VALUE_LAMBDA:
			return a == other.a;
	}

	return false;
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

void Scope::set(u32 key, Value value) {
	values.data[key] = value;
}

Value Scope::get(u32 key) {
	return values.data[key];
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
	auto b = stack->pop();
	auto a = stack->pop();

	Value c;

	if((a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) || (a.type == VALUE_REFERENCE && b.type == VALUE_REFERENCE)) {
		c.type = a.number == b.number ? VALUE_TRUE : VALUE_FALSE;
	} else if(a.type == VALUE_STRING && b.type == VALUE_STRING) {
		if(strcmp(a.string, b.string) == 0) {
			c.type = VALUE_TRUE;
		} else {
			c.type = VALUE_FALSE;
		}
	} else if((a.type == VALUE_TRUE && b.type == VALUE_TRUE) || (a.type == VALUE_FALSE && b.type == VALUE_FALSE) || (a.type == VALUE_NIL && b.type == VALUE_NIL)) {
		c.type = VALUE_TRUE;
	} else if(a.type == VALUE_OBJECT && b.type == VALUE_OBJECT) {
		// TODO
		assert(false);
	} else if(a.type == VALUE_LAMBDA && b.type == VALUE_LAMBDA) {
		// TODO
		assert(false);
	} else {
		assert(false);
	}

	stack->push(c);
}

void __rt_lt(Stack *stack) {
	auto b = stack->pop();
	auto a = stack->pop();

	Value c;

	if((a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) || (a.type == VALUE_REFERENCE && b.type == VALUE_REFERENCE)) {
		c.type = a.number < b.number ? VALUE_TRUE : VALUE_FALSE;
	} else {
		assert(false);
	}

	stack->push(c);
}

void __rt_gt(Stack *stack) {
	auto b = stack->pop();
	auto a = stack->pop();

	Value c;

	if((a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) || 
		(a.type == VALUE_REFERENCE && b.type == VALUE_REFERENCE)) {
		c.type = a.number > b.number ? VALUE_TRUE : VALUE_FALSE;
	} else {
		assert(false);
	}

	stack->push(c);
}

void __rt_cond_exec(Heap *heap, Scope *scope, Stack *stack) {
	auto body = stack->pop();
	auto cond = stack->pop();

	assert(body.type == VALUE_LAMBDA);

	if(cond.type == VALUE_TRUE || 
		cond.type == VALUE_OBJECT || 
		cond.type == VALUE_STRING || 
		cond.type == VALUE_REFERENCE || 
		cond.type == VALUE_NUMBER || 
		cond.type == VALUE_NUMBER) {
		(*body.lambda.fn)(heap, scope, stack);
	}
}

void __rt_exec(Heap *heap, Scope *scope, Stack *stack) {
	auto lambda = stack->pop();

	assert(lambda.type == VALUE_LAMBDA);

	(*lambda.lambda.fn)(heap, scope, stack);
}

void __rt_and(Stack *stack) {
	auto b = stack->pop();
	auto a = stack->pop();

	Value c;

	if((a.type == VALUE_TRUE || a.type == VALUE_FALSE) && (b.type == VALUE_TRUE || b.type == VALUE_FALSE)) {
		c.type = a.type == VALUE_TRUE && b.type == VALUE_TRUE ? VALUE_TRUE : VALUE_FALSE;
	} else if(a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) {
		c.type = VALUE_NUMBER;
		c.number = a.number & b.number;
	} else {
		assert(false);
	}

	stack->push(c);
}

void __rt_or(Stack *stack) {
	auto b = stack->pop();
	auto a = stack->pop();

	Value c;

	if((a.type == VALUE_TRUE || a.type == VALUE_FALSE) && (b.type == VALUE_TRUE || b.type == VALUE_FALSE)) {
		c.type = a.type == VALUE_TRUE || b.type == VALUE_TRUE ? VALUE_TRUE : VALUE_FALSE;
	} else if(a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) {
		c.type = VALUE_NUMBER;
		c.number = a.number | b.number;
	} else {
		assert(false);
	}

	stack->push(c);
}

void __rt_not(Stack *stack) {
	auto a = stack->pop();

	Value c;

	if(a.type == VALUE_TRUE) {
		c.type = VALUE_FALSE;
	} else if(a.type == VALUE_FALSE) {
		c.type = VALUE_TRUE;
	} else {
		assert(false);
	}

	stack->push(c);
}

void __rt_add(Stack *stack) {
	auto b = stack->pop();
	auto a = stack->pop();

	Value c;

	if(a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) {
		c.type = VALUE_NUMBER;
		c.number = a.number + b.number;
	} else if(a.type == VALUE_STRING || b.type == VALUE_STRING) {
		// TODO: string concatenation
		assert(false);
	}

	stack->push(c);
}

void __rt_sub(Stack *stack) {
	auto b = stack->pop();
	auto a = stack->pop();

	Value c;

	if(a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) {
		c.type = VALUE_NUMBER;
		c.number = a.number - b.number;
	} else {
		assert(false);
	}

	stack->push(c);
}

void __rt_mul(Stack *stack) {
	auto b = stack->pop();
	auto a = stack->pop();

	Value c;

	if(a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) {
		c.type = VALUE_NUMBER;
		c.number = a.number * b.number;
	} else {
		assert(false);
	}

	stack->push(c);
}

void __rt_div(Stack *stack) {
	auto b = stack->pop();
	auto a = stack->pop();

	Value c;

	if(a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) {
		c.type = VALUE_NUMBER;
		c.number = a.number / b.number;
	} else {
		assert(false);
	}

	stack->push(c);
}

void __rt_neg(Stack *stack) {
	auto a = stack->pop();

	Value c;

	if(a.type == VALUE_NUMBER) {
		c.type = VALUE_NUMBER;
		c.number = -a.number;
	} else {
		assert(false);
	}

	stack->push(c);
}

void __rt_mod(Stack *stack) {
	auto b = stack->pop();
	auto a = stack->pop();

	Value c;

	if(a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) {
		c.type = VALUE_NUMBER;
		c.number = a.number % b.number;
	} else {
		assert(false);
	}

	stack->push(c);
}

void __rt_newobj(Stack *stack, Heap *heap) {
	auto v = heap->alloc();
	v.type = VALUE_OBJECT;
	v.object = new Hash_Table<Value, Value>(__value_hash);
	stack->push(v);
}

void __rt_get_prop(Stack *stack) {
	auto value = stack->pop();
	auto key = stack->pop();
	auto object = stack->pop();

	assert(object.type == VALUE_OBJECT);

	object.object->put(key, value);
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
	Value v; // TODO: allocate on Heap?
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

void __rt_push_lambda(Stack *stack, Heap *heap, jit *j, lambda_fn fn) {
	auto v = heap->alloc();
	v.type = VALUE_LAMBDA;
	v.lambda.j = j;
	v.lambda.fn = fn;
	stack->push(v);
}