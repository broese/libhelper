#include <lh_debug.h>
#include "lhtest.h"

TF(warn, "warnings") {
} _TF

TF(error, "errors") {
} _TF

TF(here, "LH_HERE tracing") {
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
