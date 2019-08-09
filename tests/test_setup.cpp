#define XSTR(x) #x
#define STR(x) XSTR(x)
#define SETUP_TEST(file, name) extern void _##name(); __setup_test(STR(file), STR(name), _##name)
extern void __setup_test(const char *file, const char *name, void (*fn)());
void test_setup() {
SETUP_TEST(array_tests, push_works);
SETUP_TEST(array_tests, pop_works);
SETUP_TEST(array_tests, clear_works);
SETUP_TEST(array_tests, unordered_remove_works);
SETUP_TEST(array_tests, ordered_remove_works);
SETUP_TEST(array_tests, index_of_works);
SETUP_TEST(array_tests, expands_as_needed);
SETUP_TEST(array_tests, for_macro_works);
}
