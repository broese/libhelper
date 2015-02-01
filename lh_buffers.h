/*! \file
 * Macros for handling buffers and arrays
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////

#define lh_align(n,a) ((__typeof__(n))((((__typeof__(n))(a)-1)|((n)-1))+1))

#define lh_resize(ptr, num) ptr = realloc(ptr, num*sizeof(*ptr));

#define lh_move(ptr, from, to, num)                             \
    memmove((ptr)+(to), (ptr)+(from), (num)*sizeof(*(ptr)));

#define lh_free(ptr) { if (ptr) free(ptr); ptr=NULL; }

////////////////////////////////////////////////////////////////////////////////
/// @name Macros for zeroing memory

/*! \brief Clear a single object (non-pointer)
 * \param obj Name of the object
 */
#define lh_clear_obj(obj)               lh_clear_ptr(&(obj))

/*! \brief Clear a single object by pointer
 * \param ptr Pointer
 */
#define lh_clear_ptr(ptr)               lh_clear_num(ptr,1)

/*! \brief Clear an array of objects
 * \param ptr Array name or a pointer
 * \param num Number of elements to clear
 */
#define lh_clear_num(ptr,num) memset((ptr), 0, sizeof(*(ptr))*(num))

/*! \brief Clear a range of elements in an array
 * \param ptr Array name or a pointer
 * \param from Index of the first element to be cleared
 * \param num Number of elements to clear
 */
#define lh_clear_range(ptr, from, num)  lh_clear_num(ptr+from, num)

////////////////////////////////////////////////////////////////////////////////

#define _lh_setgran(ptr,cnt,gran)           \
    lh_resize((ptr),lh_align((cnt),(gran)))                        

#define _lh_setgran_c(ptr,cnt,gran) {                                   \
        lh_resize((ptr),lh_align((cnt),(gran)));                        \
        lh_clear_range(ptr,cnt,lh_align((cnt),(gran))-cnt);             \
    }

#define lh_setgran(...)   _lh_setgran(__VA_ARGS__)
#define lh_setgran_c(...) _lh_setgran_c(__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////
/** @name Allocation of objects, arrays and buffers
 * NOTE: all allocation macros will clear the resulting array buffer or object
 *
 * lh_create_* : Allocate and place in a new pointer variable
 *
 * lh_alloc_* : Allocate and put pointer to a variable
 */

/*! \brief Allocate a single object to a new variable
 * \param type Type of object
 * \param name Name of the new variable that will be assigned the pointer
 */
#define lh_create_obj(type,name)        lh_create_num(type,name,1)

/*! \brief Allocate a byte buffer.
 * \param name Name of the new variable that will be assigned the pointer
 * \param size Size of the buffer, in bytes
 */
#define lh_create_buf(name,size)        lh_create_num(uint8_t,name,size)

/*! \brief Allocate an array of elements
 * \param type Type of objects in the array
 * \param name Name of the new variable that will be assigned the pointer
 * \param num Number of elements in the allocated array
 */
#define lh_create_num(type,name,num)    type * lh_alloc_num(name,num)

/*! \brief Allocate single object to an existing variable
 * \param ptr Pointer varable for the object
 */
#define lh_alloc_obj(ptr)               lh_alloc_num(ptr,1)

/*! \brief Allocate a byte buffer to an existing variable
 * \param ptr Pointer varable for the buffer
 * \param size Size of the buffer, in bytes
 */
#define lh_alloc_buf(ptr,size)          lh_alloc_num(ptr,size)

/*! \brief Allocate an array of elements to an existing variable
 * \param ptr Pointer varable for the array
 * \param num Number of elements in the allocated array
 */
#define lh_alloc_num(ptr,num)                   \
    ptr = malloc((num)*sizeof(*(ptr)));         \
    lh_clear_num((ptr),(num));

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
