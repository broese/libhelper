#pragma once

/*
lh_debug - functions supporting debug output and error messages
*/

#include <stdlib.h>
#include <string.h>
#include <errno.h>

////////////////////////////////////////////////////////////////////////////////

/*

Define what method Libhelper should use when encountering errors.
The method is defined by LH_ERR. The user either undefs LH_ERR,
which results in default behavior, or defines it as one of the
constants below.

Default method (#ifndef LH_ERR):
LH returns defined error value for the function, e.g. a NULL for
pointers or a negative value for integers. Does not print any
errors. The user must analyze the return value and query the
actual error through errno;

Message (LH_ERR_MESSAGE):
LH prints an error message to STDERR and returns an error value
to the caller.

Abort (LH_ERR_ABORT):
LH prints an error message to STDERR and then aborts the execution
via exit(1).

*/

#define LH_ERR_MESSAGE 1
#define LH_ERR_ABORT   2

// TODO: come up with a concept how this should be handled for a compiled code
#define LH_ERR LH_ERR_MESSAGE

#ifndef LH_ERR

#define LH_ERROR(retval,msg,...)                \
    return retval;

#elif (LH_ERR==LH_ERR_MESSAGE)

#define LH_ERROR(retval,msg,...) {                              \
        fprintf(stderr,msg " : %s\n",##__VA_ARGS__,strerror(errno));  \
        return retval;                                          \
    }
    
#elif (LH_ERR==LH_ERR_ABORT)

#define LH_ERROR(retval,msg,...) {                              \
        fprintf(stderr,msg " : %s\n",##__VA_ARGS__,strerror(errno));  \
        exit(1);                                                \
    }

#endif


#define LH_DEBUG(msg,...)                               \
    fprintf(stderr, "%s:%d (%s) : " msg "\n",           \
            __FILE__,__LINE__,__func__,##__VA_ARGS__);

////////////////////////////////////////////////////////////////////////////////

void hexdump(const unsigned char * data, ssize_t length);
void hexprint(const unsigned char * data, ssize_t length);
