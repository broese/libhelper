#pragma once
#include <stdio.h>

#define TF(name,descr)                                      \
    static int test_##name() {                                     \
        printf("\n\n=== Testing %s ===\n", descr);    \
        int fail = 0;

#define _TF                                                 \
        printf("-----\ntotal: %s\n", PASSFAIL(!fail));      \
        return fail;                                        \
    }

#define TM(name)                                           \
    int test_module_##name() {                      \
        printf("\n\n====== Testing module %s ======\n", #name);     \
        int fail = 0;

#define _TM                                                            \
        printf("-----\nmodule total: %s\n", PASSFAIL(!fail));          \
        return fail;                                                   \
    }

#define TEST(name) fail += test_##name();

#define PASSFAIL(cond) ( (cond) ? "\x1b[32mPASS\x1b[0m" : "\x1b[31mFAIL\x1b[0m" )

