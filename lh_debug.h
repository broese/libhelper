/*
 Authors:
 Copyright 2012-2015 by Eduard Broese <ed.broese@gmx.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version
 2 of the License, or (at your option) any later version.
*/

#pragma once

/*
 Authors:
 Copyright 2012-2015 by Eduard Broese <ed.broese@gmx.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version
 2 of the License, or (at your option) any later version.

 lh_debug : functions for debug and error messages
*/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////

/*

LH_ERROR(value, msg, arg...)
    Produce an error message and bail out with given error code.

LH_ERRORV(msg, arg)
    Produce an error message and bail out with no specific code.
    Use in void functions to avoid compiler warnings

value  : error code/value to return (any type)
msg    : printf format string with the message
arg... : zero or more values to be included by the printf

The exact behavior is determined by the value of LH_ERROR_BEHAVIOR:
  LH_ERROR_QUIET   : do nothing
  LH_ERROR_MESSAGE : only produce message
  LH_ERROR_RETURN  : produce message on STDERR and return from function
  LH_ERROR_EXIT    : produce message and abort program

By default, the module will use LH_ERR_RETURN. If you want to change this
behavior, set the appropriate value before including the module, for example:

#define LH_ERROR_BEHAVIOR LH_ERROR_EXIT
#include <lh_debug.h>

*/

#define LH_ERROR_QUIET    0
#define LH_ERROR_MESSAGE  1
#define LH_ERROR_RETURN   2
#define LH_ERROR_EXIT     3

#ifndef LH_ERROR_BEHAVIOR
#define LH_ERROR_BEHAVIOR LH_ERROR_RETURN
#endif


#if (LH_ERROR_BEHAVIOR==LH_ERROR_QUIET)

#define LH_ERROR(retval,msg,...) do { } while(0);
#define LH_ERRORV(msg,...) LH_ERROR(0, msg, ##__VA_ARGS__)

#elif (LH_ERROR_BEHAVIOR==LH_ERROR_MESSAGE)

#define LH_ERROR(retval,msg,...) {                                      \
        fprintf(stderr,msg " : %s\n",##__VA_ARGS__,strerror(errno));    \
    }
#define LH_ERRORV(msg,...) LH_ERROR(0, msg, ##__VA_ARGS__)

#elif (LH_ERROR_BEHAVIOR==LH_ERROR_RETURN)

#define LH_ERROR(retval,msg,...) {                                      \
        fprintf(stderr,msg " : %s\n",##__VA_ARGS__,strerror(errno));    \
        return retval;                                                  \
    }
#define LH_ERRORV(msg,...) {                                            \
        fprintf(stderr,msg " : %s\n",##__VA_ARGS__,strerror(errno));    \
        return;                                                         \
    }

#elif (LH_ERROR_BEHAVIOR==LH_ERROR_EXIT)

#define LH_ERROR(retval,msg,...) {                                      \
        fprintf(stderr,msg " : %s\n",##__VA_ARGS__,strerror(errno));    \
        exit((int)retval);                                              \
    }
#define LH_ERRORV(msg,...) {                                            \
        fprintf(stderr,msg " : %s\n",##__VA_ARGS__,strerror(errno));    \
        exit(1);                                                        \
    }

#endif

////////////////////////////////////////////////////////////////////////////////

/*
  Print a debug message of defined level.
  The debug level is defined in an int variable defined by LH_DEBUG_LEVEL_NAME,
  by default: lh_debug_level.

  LH_DEBUG(level, msg, arg...);

  level  : level of the message
  msg    : printf format string with the message
  arg... : zero or more values to be included by the printf

  LH_INFO(msg, arg...)
  LH_WARN(msg, arg...)
  LH_ATTN(msg, arg...)
  LH_ALERT(msg, arg...)

  Short-form wrapper macros
 */

#ifndef LH_DEBUG_LEVEL_NAME
#define LH_DEBUG_LEVEL_NAME lh_debug_level
#endif

extern int LH_DEBUG_LEVEL_NAME;

#define LH_DEBUG_LEVEL_INFO  0
#define LH_DEBUG_LEVEL_WARN  1
#define LH_DEBUG_LEVEL_ATTN  2
#define LH_DEBUG_LEVEL_ALERT 3

#define LH_DEBUG(level,msg,...)                             \
    if (level >= LH_DEBUG_LEVEL_NAME)                       \
        fprintf(stderr, "%s:%d (%s) : " msg "\n",           \
                __FILE__,__LINE__,__func__,##__VA_ARGS__);

#define LH_INFO(msg,...)  LH_DEBUG(LH_DEBUG_LEVEL_INFO,msg,##__VA_ARGS__)
#define LH_WARN(msg,...)  LH_DEBUG(LH_DEBUG_LEVEL_WARN,msg,##__VA_ARGS__)
#define LH_ATTN(msg,...)  LH_DEBUG(LH_DEBUG_LEVEL_ATTN,msg,##__VA_ARGS__)
#define LH_ALERT(msg,...) LH_DEBUG(LH_DEBUG_LEVEL_ALERT,msg,##__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////

#define LH_HERE {                                                       \
        fprintf(stderr, "%s:%d (%s)\n", __FILE__,__LINE__,__func__);    \
        fflush(stderr);                                                 \
    }

////////////////////////////////////////////////////////////////////////////////

void hexdump(const void * data, ssize_t length);
void hexprint(const void * data, ssize_t length);
ssize_t hex_import(const char *hex, uint8_t *bin, ssize_t maxlen);
