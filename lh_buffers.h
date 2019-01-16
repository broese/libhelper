#pragma once

/*
 Authors:
 Copyright 2012-2015 by Eduard Broese <ed.broese@gmx.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version
 2 of the License, or (at your option) any later version.

 lh_buffers : buffer/objects allocation
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
/// Basic macros

/*
  lh_align(n,a)
  Calculate next number larger or equal to number 'n' and aligned to 'a'
  alignment value must be a power of 2

  lh_resize(ptr, num)
  Resize the allocated space for ptr to accomodate for num objects
  using realloc()

  lh_move(ptr, from, to, num)
  Move 'num' elements within array pointed to by 'ptr' from position 'from'
  to position 'to' using memove.

  lh_free(ptr)
  Free allocated memory at pointer 'ptr' and set the pointer variable to NULL,
  but only if 'ptr' is not NULL
*/
#define lh_align(n,a) ((__typeof__(n))((((__typeof__(n))(a)-1)|((n)-1))+1))

#define lh_resize(ptr, num) ptr = realloc(ptr, (num)*sizeof(*(ptr)));

#define lh_move(ptr, from, to, num)                             \
    memmove((ptr)+(to), (ptr)+(from), (num)*sizeof(*(ptr)));

#define lh_free(ptr) { if (ptr) free(ptr); ptr=NULL; }

////////////////////////////////////////////////////////////////////////////////
/// Macros for zeroing memory

/*
  lh_clear_obj(obj)
  clear a single object 'obj' (non-pointer)

  lh_clear_ptr(ptr)
  clear a single object referenced by pointer 'ptr'

  lh_clear_num(ptr,num)
  clear 'num' elements in an array referenced by the pointer 'ptr'

  lh_clear_range
  clear 'num' elements in an array referenced by the pointer 'ptr' starting
  with element at index 'from'
*/

#define lh_clear_obj(obj)               lh_clear_ptr(&(obj))
#define lh_clear_ptr(ptr)               lh_clear_num(ptr,1)
#define lh_clear_num(ptr,num)           memset((ptr), 0, sizeof(*(ptr))*(num))
#define lh_clear_range(ptr, from, num)  lh_clear_num((ptr)+(from), num)

////////////////////////////////////////////////////////////////////////////////
/// Granularity

/*
  lh_setgran(ptr,cnt,gran)
  ensure that the array referenced by the pointer 'ptr' and containing 'cnt'
  elements is allocated with granularity 'gran'

  lh_setgran_c(ptr,cnt,gran)
  same as lh_setgran, but additionally clear the elements in the granularity
  padding
*/
#define _lh_setgran(ptr,cnt,gran)           \
    lh_resize((ptr),lh_align((cnt),(gran)))                        

#define _lh_setgran_c(ptr,cnt,gran) {                                   \
        lh_resize((ptr),lh_align((cnt),(gran)));                        \
        lh_clear_range(ptr,cnt,lh_align((cnt),(gran))-(cnt));           \
    }

#define lh_setgran(...)   _lh_setgran(__VA_ARGS__)
#define lh_setgran_c(...) _lh_setgran_c(__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
/// Allocation of objects, arrays and buffers

/*
  NOTE: all allocation macros will clear the resulting array buffer or object

  lh_create_obj(type,name)
  allocate a single object of type 'type' and place the pointer into a new
  pointer variable 'name'

  lh_create_buf(name,size)
  allocate a buffer (uint8_t *) of size 'size' and store the pointer into a
  new variable 'name'

  lh_create_num(type,name,num)
  Allocate 'num' elements of type 'type' and store the pointer into a new
  variable 'name'

  lh_alloc_obj(ptr)
  Allocate a single element and place the pointer into existing variable 'ptr'

  lh_alloc_buf(ptr,size)
  Allocate a buffer of size 'size' and place the pointer into existing
  variable 'ptr'

  lh_alloc_num(ptr,num)
  Allocate an array with 'num' elements and place the pointer into
  existing variable 'ptr'

*/

#define lh_create_obj(type,name)        lh_create_num(type,name,1)
#define lh_create_buf(name,size)        lh_create_num(uint8_t,name,size)
#define lh_create_num(type,name,num)    type * lh_alloc_num(name,num)
#define lh_alloc_obj(ptr)               lh_alloc_num(ptr,1)
#define lh_alloc_buf(ptr,size)          lh_alloc_num(ptr,size)
#define lh_alloc_num(ptr,num)           ptr = calloc((num), sizeof(*(ptr)));

////////////////////////////////////////////////////////////////////////////////

#ifdef LH_DECLARE_SHORT_NAMES

#define ALIGN                           lh_align
#define FREE                            lh_free

#define CLEAR                           lh_clear_obj
#define CLEARP                          lh_clear_ptr
#define CLEARN                          lh_clear_num
#define CLEAR_RANGE                     lh_clear_range

#define CREATE                          lh_create_obj
#define CREATEN                         lh_create_num
#define CREATEB                         lh_create_buf

#define ALLOC                           lh_alloc_obj
#define ALLOCN                          lh_alloc_num
#define ALLOCB                          lh_alloc_buf

#define RESIZE                          lh_resize
#define MOVE                            lh_move
#define setgran                         lh_setgran
#define setgran_c                       lh_setgran_c

#endif
