#ifdef TESTING

#include "tests.h"
#include "array.h"

DEFINE_TEST(push_works) {
	auto a = Array<u32>();
	
	for(u32 i = 0; i < 10; i++) a.push(i);

	assert(a.count == 10);
	for(u32 i = 0; i < 10; i++) assert(a.data[i] == i);
}

DEFINE_TEST(pop_works) {

}

#endif