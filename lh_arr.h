#pragma once

/*
 Authors:
 Copyright 2012-2015 by Eduard Broese <ed.broese@gmx.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version
 2 of the License, or (at your option) any later version.

 lh_arr : resizable arrays
*/

////////////////////////////////////////////////////////////////////////////////
/**
 * \file Resizable Arrays
 * Macros for handling expandable arrays and buffers.
 *
 * The resizable arrays are defined by two variables - a pointer variable \c ptr
 * and the integer counter variable \c cnt. The macros perform the allocation
 * and resizing of the arrays. Allocation granularity is defined in a separate
 * parameter.
 *
 * A better usability (reduced number of parameters) can be achieved
 * through special macros controlling default granularity and naming scheme.
 * Consider these examples:
 *
 * lh_array_allocate(ptr,cnt,4096,num)
 * - specify variables and parameters explicitly
 *
 * lh_array_allocate(AR(name),4096,num)
 * - use the standard naming scheme, implies variables p_name and c_name
 *
 * lh_array_allocate(GAR3(name),num)
 * - use the standard naming scheme and granularity 4096
 *
 * lh_array_allocate(GAR(name),num)
 * - use the standard naming scheme and default granularity (LH_DEFAULT_GRAN)
 *
 * If a different naming or granularity scheme should be used, the user
 * shall define his own macros similar to AR()
 *
 * NOTE: You <b>must always</b> use same granularity when allocating and
 * reallocating an array throughout its lifetime. If you increase the
 * granularity, you risk memory corruption as the fucntions will make a
 * wrong assumption about the allocated size. \c setgran macros can be used
 * to ensuire specific granularity on an array.
 *
 * The operations on the array (allocate, resize, add, etc) come in two kinds -
 * the standard named macro does not clear the newly allocated elements, and
 * the macros with the _c suffix do.
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "lh_buffers.h"

////////////////////////////////////////////////////////////////////////////////

#ifndef LH_DEFAULT_GRAN
#define LH_DEFAULT_GRAN 256
#endif

#define P(name)  name##_p
#define C(name)  name##_c
#define G(name)  name##_g
#define AR(name) P(name),C(name)

#define GAR(name) AR(name),LH_DEFAULT_GRAN

#define GAR0(name) AR(name),1
#define GAR1(name) AR(name),(1<<4)
#define GAR2(name) AR(name),(1<<8)
#define GAR3(name) AR(name),(1<<12)
#define GAR4(name) AR(name),(1<<16)
#define GAR5(name) AR(name),(1<<20)
#define GAR6(name) AR(name),(1<<24)

////////////////////////////////////////////////////////////////////////////////

#define lh_arr_declare(type,name)   type * P(name); ssize_t C(name);
#define lh_arr_declare_i(type,name) type * P(name)=NULL; ssize_t C(name)=0;

#define lh_buf_declare(name)        lh_arr_declare(uint8_t,name)
#define lh_buf_declare_i(name)      lh_arr_declare_i(uint8_t,name)

#define _lh_arr_init(ptr,cnt,...)   ptr=NULL; cnt=0;
#define _lh_arr_free(ptr,cnt,...)   { if (ptr) free(ptr); lh_arr_init(ptr,cnt); }

#define lh_arr_init(...)            _lh_arr_init(__VA_ARGS__)
#define lh_arr_free(...)            _lh_arr_free(__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////

static inline void * lh_arr_insert_range_(
    void ** ptr,
    ssize_t *cnt,
    ssize_t size,
    ssize_t gran,
    ssize_t idx,
    ssize_t num) {

    // check array bounds
    assert(idx >= 0);
    assert(idx <= *cnt); //NOTE: idx=*cnt means adding element at the end

    ssize_t newcnt = (*cnt+num);

    // allocate more memory if needed
    if (lh_align(newcnt,gran) > lh_align(*cnt,gran))
        *ptr = realloc(*ptr, lh_align(newcnt,gran)*size);

    // move data to provide space for the new elements
    ssize_t idxpos = idx*size;
    ssize_t newpos = (idx+num)*size;
    ssize_t mvsize = (*cnt-idx)*size;

    if (mvsize > 0)
        memmove(*ptr+newpos, *ptr+idxpos, mvsize);

    // update array size
    *cnt = newcnt;

    // return pointer to the first inserted element
    return *ptr+idxpos;
}

static inline void * lh_arr_delete_range_(
    void ** ptr,
    ssize_t *cnt,
    ssize_t size,
    ssize_t gran,
    ssize_t idx,
    ssize_t num) {

    // check array bounds
    assert(idx >= 0);
    assert(idx < *cnt); //NOTE: cannot delete at the end
    assert(idx+num <= *cnt);

    // move data to close the gap
    ssize_t idxpos = idx*size;
    ssize_t newpos = (idx+num)*size;
    ssize_t mvsize = (*cnt-idx-num)*size;

    if (mvsize > 0)
        memmove(*ptr+idxpos, *ptr+newpos, mvsize);

    // update array size
    *cnt-=num;

    // return pointer to the first element after the array end
    return *ptr+*cnt*size;
}

////////////////////////////////////////////////////////////////////////////////

#define _lh_arr_insert_range(ptr,cnt,gran,idx,num)                       \
    (__typeof__(ptr)) lh_arr_insert_range_((void **)&(ptr),&(cnt),sizeof(*(ptr)),gran,idx,num)
#define _lh_arr_insert_range_c(ptr,cnt,gran,idx,num)                     \
    (__typeof__(ptr)) memset(_lh_arr_insert_range(ptr,cnt,gran,idx,num),0,sizeof(*(ptr))*num)

#define _lh_arr_insert(ptr,cnt,gran,idx)        \
    _lh_arr_insert_range(ptr,cnt,gran,idx,1)
#define _lh_arr_insert_c(ptr,cnt,gran,idx)      \
    _lh_arr_insert_range_c(ptr,cnt,gran,idx,1)

#define _lh_arr_add(ptr,cnt,gran,num)           \
    _lh_arr_insert_range(ptr,cnt,gran,cnt,num)
#define _lh_arr_add_c(ptr,cnt,gran,num)         \
    _lh_arr_insert_range_c(ptr,cnt,gran,cnt,num)

#define _lh_arr_new(ptr,cnt,gran)               \
    _lh_arr_insert_range(ptr,cnt,gran,cnt,1)
#define _lh_arr_new_c(ptr,cnt,gran)             \
    _lh_arr_insert_range_c(ptr,cnt,gran,cnt,1)

#define _lh_arr_delete_range(ptr,cnt,gran,idx,num)                      \
    (__typeof__(ptr)) lh_arr_delete_range_((void **)&(ptr),&(cnt),sizeof(*(ptr)),gran,idx,num)
#define _lh_arr_delete_range_c(ptr,cnt,gran,idx,num)                    \
    (__typeof__(ptr)) memset(_lh_arr_delete_range(ptr,cnt,gran,idx,num),0,sizeof(*(ptr))*num)

#define _lh_arr_delete(ptr,cnt,gran,idx)        \
    _lh_arr_delete_range(ptr,cnt,gran,idx,1)
#define _lh_arr_delete_c(ptr,cnt,gran,idx)      \
    _lh_arr_delete_range_c(ptr,cnt,gran,idx,1)

#define _lh_arr_allocate(ptr,cnt,gran,num)                                 \
    ptr = (__typeof__(ptr)) malloc(lh_align(((cnt)=(num)),gran)*sizeof(*(ptr)))
#define _lh_arr_allocate_c(ptr,cnt,gran,num)                               \
    ptr = (__typeof__(ptr)) calloc(lh_align(((cnt)=(num)),gran),sizeof(*(ptr)))

#define _lh_arr_resize(ptr,cnt,gran,num)                    \
    (((cnt)<=(num)) ?                                       \
     _lh_arr_add(ptr,cnt,gran,(num)-(cnt)) :                \
     _lh_arr_delete_range(ptr,cnt,gran,num,(cnt)-(num)))
#define _lh_arr_resize_c(ptr,cnt,gran,num)                  \
    (((cnt)<=(num)) ?                                       \
     _lh_arr_add_c(ptr,cnt,gran,(num)-(cnt)) :              \
     _lh_arr_delete_range_c(ptr,cnt,gran,num,(cnt)-(num)))

#define lh_arr_insert_range(...)   _lh_arr_insert_range(__VA_ARGS__)
#define lh_arr_insert_range_c(...) _lh_arr_insert_range_c(__VA_ARGS__)
#define lh_arr_insert(...)         _lh_arr_insert(__VA_ARGS__)
#define lh_arr_insert_c(...)       _lh_arr_insert_c(__VA_ARGS__)
#define lh_arr_add(...)            _lh_arr_add(__VA_ARGS__)
#define lh_arr_add_c(...)          _lh_arr_add_c(__VA_ARGS__)
#define lh_arr_new(...)            _lh_arr_new(__VA_ARGS__)
#define lh_arr_new_c(...)          _lh_arr_new_c(__VA_ARGS__)

#define lh_arr_delete_range(...)   _lh_arr_delete_range(__VA_ARGS__)
#define lh_arr_delete_range_c(...) _lh_arr_delete_range_c(__VA_ARGS__)
#define lh_arr_delete(...)         _lh_arr_delete(__VA_ARGS__)
#define lh_arr_delete_c(...)       _lh_arr_delete_c(__VA_ARGS__)

#define lh_arr_allocate(...)       _lh_arr_allocate(__VA_ARGS__)
#define lh_arr_allocate_c(...)     _lh_arr_allocate_c(__VA_ARGS__)
#define lh_arr_resize(...)         _lh_arr_resize(__VA_ARGS__)
#define lh_arr_resize_c(...)       _lh_arr_resize_c(__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////

#ifdef LH_DECLARE_SHORT_NAMES

#define ARR                        lh_arr_declare
#define ARRI                       lh_arr_declare_i
#define BUF                        lh_buf_declare
#define BUFI                       lh_buf_declare_i

#define arr_init                   lh_arr_init
#define arr_free                   lh_arr_free

#define arr_add                    lh_arr_add
#define arr_add_c                  lh_arr_add_c
#define arr_new                    lh_arr_new
#define arr_new_c                  lh_arr_new_c
#define arr_ins                    lh_arr_insert
#define arr_ins_c                  lh_arr_insert_c
#define arr_insr                   lh_arr_insert
#define arr_insr_c                 lh_arr_insert_c

#define arr_del                    lh_arr_delete
#define arr_del_c                  lh_arr_delete_c
#define arr_delr                   lh_arr_delete_range
#define arr_delr_c                 lh_arr_delete_range_c

#define arr_alloc                  lh_arr_allocate
#define arr_alloc_c                lh_arr_allocate_c
#define arr_resize                 lh_arr_resize
#define arr_resize_c               lh_arr_resize_c

#endif
