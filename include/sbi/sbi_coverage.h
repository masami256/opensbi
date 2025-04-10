#ifndef FUZZNIG_COVERAGE
#define FUZZNIG_COVERAGE

void __cyg_profile_func_enter(void *this_fn, void *call_site);

void __cyg_profile_func_exit(void *this_fn, void *call_site);

#endif // FUZZNIG_COVERAGE