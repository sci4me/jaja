extern "C" {
#include "myjit/jitlib.h"
}

#include "types.h"

typedef void (*fn)();

s32 main(s32 argc, char **argv) {
	auto p = jit_init();

	fn f;
	jit_prolog(p, &f);
	jit_msg(p, "Hello, World!\n");
	jit_reti(p, 0);
	jit_generate_code(p);

	f();

	jit_free(p);
	return 0;
}