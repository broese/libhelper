#pragma once

#ifdef __linux__

//#define HAVE_BUILTIN_BSWAP16 1
#define HAVE_BUILTIN_BSWAP32 1
#define HAVE_BUILTIN_BSWAP64 1

#define HAVE_QSORT_R 1
#define HAVE_STRNLEN 1
#define HAVE_BLKGETSIZE64 1

#define HAVE_OPENSSL 1

#endif




#ifdef __sun__

#define HAVE_TELL

#endif


#ifdef __CYGWIN__

#include <sys/types.h>
#define HAVE_OPENSSL 1

#endif
