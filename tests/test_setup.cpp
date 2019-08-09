#define XSTR(x) #x
#define STR(x) XSTR(x)
#define SETUP_TEST(name) __setup_test(STR(name), _##name)
extern void __setup_test(const char *name, void (*fn)());
void test_setup() {
extern void _push_works(); SETUP_TEST(push_works);
extern void _pop_works(); SETUP_TEST(pop_works);
extern void _clear_works(); SETUP_TEST(clear_works);
extern void _unordered_remove_works(); SETUP_TEST(unordered_remove_works);
extern void _ordered_remove_works(); SETUP_TEST(ordered_remove_works);
extern void _index_of_works(); SETUP_TEST(index_of_works);
extern void _expands_as_needed(); SETUP_TEST(expands_as_needed);
extern void _for_macro_works(); SETUP_TEST(for_macro_works);
}
