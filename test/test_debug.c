/*
 Authors:
 Copyright 2012-2015 by Eduard Broese <ed.broese@gmx.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version
 2 of the License, or (at your option) any later version.

 lh_debug : functions for debug and error messages
*/

#include "lhtest.h"

//#define LH_ERROR_BEHAVIOR LH_ERROR_EXIT
#define LH_DEBUG_LEVEL_NAME my_debug_level
#include <lh_debug.h>

int my_debug_level = 1;

static int debug_function(int level) {
    my_debug_level = level;
    LH_INFO("information");
    LH_WARN("warning");
    LH_ATTN("attention");
    LH_ALERT("alert");
}

TF(warn, "warnings") {
    debug_function(LH_DEBUG_LEVEL_INFO); printf("---\n");
    debug_function(LH_DEBUG_LEVEL_WARN); printf("---\n");
    debug_function(LH_DEBUG_LEVEL_ATTN); printf("---\n");
    debug_function(LH_DEBUG_LEVEL_ALERT); printf("---\n");
} _TF

static int error_function(int val) {
    errno=val;
    LH_ERROR(val+1, "OK, val=%d", val);
}

static void void_error_function(int val) {
    errno=val;
    LH_ERRORV("OK, val=%d", val);
}

TF(error, "errors") {
    fail += (error_function(1)!=2);
    fail += (error_function(2)!=3);
    fail += (error_function(3)!=4);
    void_error_function(10);
} _TF

TF(here, "LH_HERE tracing") {
    LH_HERE;
    printf("in between\n");
    LH_HERE;
} _TF

TF(hex, "hex output") {
    const char *test = "This is an example string\n";
    hexdump(test, strlen(test));
    hexprint(test, strlen(test));
} _TF

////////////////////////////////////////////////////////////////////////////////

TM(debug) {

    TEST(warn);
    TEST(error);
    TEST(here);
    TEST(hex);

} _TM;
