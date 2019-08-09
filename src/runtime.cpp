#include <string.h>

#include "runtime.h"

bool Value::is_truthy() {
	switch(type) {
		case VALUE_FALSE:	
		case VALUE_NIL:
			return false;
		case VALUE_TRUE:	
		case VALUE_NUMBER:	
		case VALUE_STRING:	
		case VALUE_REFERENCE:	
		case VALUE_OBJECT:	
		case VALUE_LAMBDA:
			return true;
	}

	assert(false);
	return false;
}

bool Value::operator==(const Value &other) {
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

void Stack::push(Value v) {
	data.push(v);
}

Value Stack::pop() {
	return data.pop();
}

Value Stack::peek() {
	assert(data.count > 0);
	return data.data[data.count - 1];
}

void Stack::set_top(Value v) {
	assert(data.count > 0);
	data.data[data.count - 1] = v;
}

void Scope::set(char *key, Value value) {
	Scope *curr = this;
	while(curr) {
		if(curr->contains(key)) {
			curr->values.put(key, value);			
			return;
		}

		curr = curr->parent;
	}

	values.put(key, value);
}

Value Scope::get(char *key) {
	Scope *curr = this;
	while(curr) {
		if(curr->contains(key)) {
			return curr->values.get(key);
		}

		curr = curr->parent;
	}

	Value v;
	v.type = VALUE_NIL;
	return v;
}

bool Scope::contains(char *key) {
	return values.contains(key);
}

void Scope::pop(Heap *heap) {
	FOR((&values), i) {
		auto v = values.values[i];
		if(v.a) heap->unmark_root(v.a);
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
			// TODO
			assert(false);
			break;
		case VALUE_LAMBDA:
			return hash_ptr(v.a);
	}

	assert(false);
	return 0;
}

static bool __value_eq(Value a, Value b) {
	return a == b;
}

void __rt_eq(Stack *stack) {
	auto b = stack->pop();
	auto a = stack->peek();

	Value c;
	c.type = a == b ? VALUE_TRUE : VALUE_FALSE;
	stack->set_top(c);
}

void __rt_lt(Stack *stack) {
	auto b = stack->pop();
	auto a = stack->peek();

	Value c;

	if((a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) || (a.type == VALUE_REFERENCE && b.type == VALUE_REFERENCE)) {
		c.type = a.number < b.number ? VALUE_TRUE : VALUE_FALSE;
	} else {
		assert(false);
	}

	stack->set_top(c);
}

void __rt_gt(Stack *stack) {
	auto b = stack->pop();
	auto a = stack->peek();

	Value c;

	if((a.type == VALUE_NUMBER && b.type == VALUE_NUMBER) || 
		(a.type == VALUE_REFERENCE && b.type == VALUE_REFERENCE)) {
		c.type = a.number > b.number ? VALUE_TRUE : VALUE_FALSE;
	} else {
		assert(false);
	}

	stack->set_top(c);
}

void __rt_cond_exec(Heap *heap, Scope *scope, Stack *stack) {
	auto body = stack->pop();
	auto cond = stack->pop();

	assert(body.type == VALUE_LAMBDA);

	if(cond.is_truthy()) {
		auto s = Scope(scope);
		if(body.a) heap->mark_root(body.a);
		(*body.lambda.fn)(heap, &s, stack);
		if(body.a) heap->unmark_root(body.a);
	}
}

void __rt_exec(Heap *heap, Scope *scope, Stack *stack) {
	auto lambda = stack->pop();

	assert(lambda.type == VALUE_LAMBDA);

	if(lambda.a) heap->mark_root(lambda.a);
	(*lambda.lambda.fn)(heap, scope, stack);
	if(lambda.a) heap->unmark_root(lambda.a);
}

void __rt_and(Stack *stack) {
	auto b = stack->pop();
	auto a = stack->peek();

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
	auto a = stack->peek();

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

	Value c;

	if(a.type == VALUE_TRUE) {
		c.type = VALUE_FALSE;
	} else if(a.type == VALUE_FALSE) {
		c.type = VALUE_TRUE;
	} else {
		assert(false);
	}

	stack->set_top(c);
}

void __rt_add(Stack *stack) {
	auto b = stack->pop();
	auto a = stack->peek();

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
	auto a = stack->peek();

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
	auto a = stack->peek();

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
	auto a = stack->peek();

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

	Value c;

	if(a.type == VALUE_NUMBER) {
		c.type = VALUE_NUMBER;
		c.number = -a.number;
	} else {
		assert(false);
	}

	stack->set_top(c);
}

void __rt_mod(Stack *stack) {
	auto b = stack->pop();
	auto a = stack->peek();

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
	v->object = (Hash_Table<Value, Value>*) malloc(sizeof(Hash_Table<Value, Value>));
	*v->object = Hash_Table<Value, Value>(__value_hash, __value_eq);
	stack->push(*v);
}

void __rt_get_prop(Stack *stack) {
	auto key = stack->pop();
	auto object = stack->pop();

	assert(object.type == VALUE_OBJECT);

	if(object.object->contains(key)) {
		stack->push(object.object->get(key));
	} else {
		Value v;
		v.type = VALUE_NIL;
		stack->push(v);
	}
}

void __rt_set_prop(Stack *stack) {
	auto value = stack->pop();
	auto key = stack->pop();
	auto object = stack->pop();

	assert(object.type == VALUE_OBJECT);

	object.object->put(key, value);
}

void __rt_dup(Stack *stack) {
	stack->push(stack->data.data[stack->data.count - 1]);	
}

void __rt_drop(Stack *stack) {
	stack->pop();
}

void __rt_swap(Stack *stack) {
	auto a = stack->pop();
	auto b = stack->pop();
	stack->push(a);
	stack->push(b);
}

void __rt_rot(Stack *stack) {
	auto a = stack->pop();
	auto b = stack->pop();
	auto c = stack->pop();
	stack->push(a);
	stack->push(c);
	stack->push(b);
}

void __rt_load(Stack *stack, Scope *scope) {
	auto key = stack->pop();

	assert(key.type == VALUE_REFERENCE);

	stack->push(scope->get(key.string));
}

void __rt_store(Heap *heap, Scope *scope, Stack *stack) {
	auto key = stack->pop();
	auto value = stack->pop();

	assert(key.type == VALUE_REFERENCE);

	auto old = scope->get(key.string);
	
	if(old.a) heap->unmark_root(old.a);
	if(value.a)	heap->mark_root(value.a);

	scope->set(key.string, value);
}

void __rt_while(Heap *heap, Scope *scope, Stack *stack) {
	auto body = stack->pop();
	auto cond = stack->pop();

	assert(body.type == VALUE_LAMBDA);
	assert(cond.type == VALUE_LAMBDA);

	if(body.a) heap->mark_root(body.a);
	if(cond.a) heap->mark_root(cond.a);

	for(;;) {
		(*cond.lambda.fn)(heap, scope, stack);

		auto v = stack->pop();
		if(!v.is_truthy()) break;

		(*body.lambda.fn)(heap, scope, stack);
	}

	if(body.a) heap->unmark_root(body.a);
	if(cond.a) heap->unmark_root(cond.a);
}

void __rt_push_true(Stack *stack) {
	Value v;
	v.a = 0;
	v.type = VALUE_TRUE;
	stack->push(v);
}

void __rt_push_false(Stack *stack) {
	Value v;
	v.a = 0;
	v.type = VALUE_FALSE;
	stack->push(v);
}

void __rt_push_nil(Stack *stack) {
	Value v;
	v.a = 0;
	v.type = VALUE_NIL;
	stack->push(v);
}

void __rt_push_number(Stack *stack, s64 n) {
	Value v;
	v.a = 0;
	v.type = VALUE_NUMBER;
	v.number = n;
	stack->push(v);
}

void __rt_push_string(Stack *stack, char *s) {
	Value v;
	v.a = 0;
	v.type = VALUE_STRING;
	v.string = s;
	stack->push(v);
}

void __rt_push_reference(Stack *stack, char *r) {
	Value v;
	v.a = 0;
	v.type = VALUE_REFERENCE;
	v.string = r;
	stack->push(v);
}

void __rt_push_lambda(Stack *stack, Heap *heap, jit *j, lambda_fn fn) {
	auto v = GC_ALLOC(heap);
	v->type = VALUE_LAMBDA;
	v->lambda.j = j;
	v->lambda.fn = fn;
	stack->push(*v);
}

void __rt_epilogue(Scope *scope, Heap *heap) {
	scope->pop(heap);
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
			// TODO
			printf("λ");
			break;
	}
}

void __std_println(Heap *heap, Scope *scope, Stack *stack) {
	__std_print(heap, scope, stack);
	printf("\n");
}