#ifndef COMPILER_H
#define COMPILER_H

#include "ast.h"
#include "runtime.h"
#include "gc.h"
#include "bitset.h"

// #define JIT_DEBUG
// #define JIT_RALLOC_TRACKING
// #define JIT_MAX_RALLOC_TRACKING

#ifdef JIT_RALLOC_TRACKING
	struct RAllocation {
		const char *func;
		const char *file;
		u32 line;
	};

	#define RALLOC() ralloc(__func__, __FILE__, __LINE__)
#else
	#define RALLOC() ralloc()
#endif

#define RFREE(x) rfree(x)

#define jit_call_method(j, m) { auto fptr = m; jit_call(j, reinterpret_cast<void *&>(fptr)); }

#define R_HEAP R(0)
#define R_SCOPE R(1)
#define R_STACK R(2)

struct Compiler {
#ifdef JIT_RALLOC_TRACKING
	Hash_Table<u64, RAllocation> *allocations;
	Array<Hash_Table<u64, RAllocation>*> allocationsStack;
#endif
#ifdef JIT_MAX_RALLOC_TRACKING
	u32 max_rallocs;
	Array<u32> max_rallocs_stack;
#endif
	Bitset *registers;
	Array<Bitset*> registersStack;
	Array<jit*> jits;

	Compiler() : registers(0) {};
	~Compiler();

#ifdef JIT_RALLOC_TRACKING
	jit_value ralloc(const char *func, const char *file, const u32 line);
#else
	jit_value ralloc();
#endif
	void rfree(jit_value r);

	Value compile(Array<Node*>* ast);
	Lambda compile_raw(Array<Node*>* ast);
	void compile_lambda(jit *j, Node *n);
	void compile_instruction(jit *j, Node *n);
	void compile_constant(jit *j, Node *n);
};

#endif