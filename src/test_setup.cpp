#define XSTR(x) #x
#define STR(x) XSTR(x)
#define SETUP_TEST(name) __setup_test(STR(name), _##name)
extern void __setup_test(const char *name, void (*fn)());
void test_setup() {
extern void _push_works(); SETUP_TEST(push_works);
extern void _pop_works(); SETUP_TEST(pop_works);
}
