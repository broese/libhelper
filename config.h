#pragma once

#ifdef __linux__

#define HAVE_MTRACE 1

//#define HAVE_BUILTIN_BSWAP16 1
#define HAVE_BUILTIN_BSWAP32 1
#define HAVE_BUILTIN_BSWAP64 1

#define HAVE_QSORT_R 1

#endif

#ifdef __sun__

#endif
