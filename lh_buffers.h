/*! \file
 * Macros for handling buffers and arrays
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
/**
 * @name Generic Macros
 */

//TODO: move generic macros into a separate module lh_generic.h

/*! \brief Align a number to the nearest boundary
 * \param num Number of elements
 * \param align Alignment size
 * \return Aligned number
 */
#define lh_align(num,align) ((((num)-1)|(((__typeof__(num))align)-1)) + 1)

////////////////////////////////////////////////////////////////////////////////
/**
 * @name Clearing Macros
 * Macros for zeroing memory
 */

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
/**
 * @name Allocation Macros
 * Allocation objects, arrays and buffers
 */

//NOTE: all allocation macros will clear the resulting array buffer or object

// lh_create_* : Allocate and place in a new pointer variable

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

// lh_alloc_* : Allocate and put pointer to a variable

/*! \brief Allocate single object to an existing variable
 * \param ptr Pointer varable for the object
 */
#define lh_alloc_obj(ptr)               lh_alloc_num(ptr,1)

/*! \brief Allocate a byte buffer to an existing variable
 * \param ptr Pointer varable for the object
 */
#define lh_alloc_buf(ptr,size)          lh_alloc_num(ptr,size)

/*! \brief Allocate an array of elements to an existing variable
 * \param ptr Pointer varable for the object
 */
#define lh_alloc_num(ptr,num)                   \
    ptr = malloc((num)*sizeof(*(ptr)));         \
    lh_clear_num((ptr),(num));


////////////////////////////////////////////////////////////////////////////////
/**
 * @name Resizable Arrays
 * Macros for handling expandable arrays and buffers.
 * The expandable arrays are defined via variable 'ptr' holding the pointer
 * to data and 'cnt' - an integer variable holding the count of elements.
 * The macros perform allocating and resizing of these arrays. Allocation
 * granularity can be additionally specified in the granular versions.
 *
 * NOTE: You <b>must always</b> use same granularity when allocating and
 * reallocating an array throughout its lifetime. If you increase the
 * granularity, you risk memory corruption as the fucntions will make a
 * wrong assumption about the allocated size.
 */

/*! \brief Resize the allocted memory of an array to a given number of elements.
 * \param ptr Pointer to allocated memory
 * \param num New number of elements to be available after allocation
 */
#define lh_resize(ptr, num) ptr = realloc(ptr, num*sizeof(*ptr));

/*! \brief Declare an expandable array. Variables are initialized but
 * no memory is llocated
 * \param type Type of elements in the array
 * \param ptr Name of the pointer variable
 * \param cnt Name of the counter variable
 */
#define lh_array(type,ptr,cnt) type * ptr=NULL; ssize_t cnt=0;

/*! \brief Declare and expandable buffer (elements are of type uint8_t).
 * \param ptr Name of the pointer variable
 * \param cnt Name of the counter variable
 */
#define lh_buffer(ptr,cnt)              lh_array(uint8_t,ptr,cnt)

/*! \brief Allocate memory for a given number of elements in an expandable array
 * \param ptr Name of the pointer variable
 * \param cnt Name of the counter variable
 * \param num Number of elements to be allocated for
 * \param gran Granularity of allocation
 */
#define lh_array_allocate_g(ptr,cnt,num,gran) {                         \
        lh_alloc_num(ptr,lh_align(num,gran));                           \
        cnt=num;                                                        \
    }

/*! \brief Resize an expandable array to a new number of elements.
 * Old elements are copied, new elements are zeroed.
 * \param ptr Name of the pointer variable
 * \param cnt Name of the counter variable
 * \param num Number of elements to be allocated for
 * \param gran Granularity of allocation, must be power of 2
 */
#define lh_array_resize_g(ptr,cnt,num,gran) {                           \
        if (lh_align((num),(gran)) > lh_align((cnt),(gran))) {          \
            lh_resize((ptr),lh_align((num),(gran)));                    \
            lh_clear_range((ptr), (cnt), (lh_align((num),gran))-(cnt)); \
        }                                                               \
        cnt=num;                                                        \
    }

/*! \brief Resize an expandable array by adding a number of elements
 * \param ptr Name of the pointer variable
 * \param cnt Name of the counter variable
 * \param num Number of elements to be added
 * \param gran Granularity of allocation, must be power of 2
 */
#define lh_array_add_g(ptr,cnt,num,gran)            \
    lh_array_resize_g(ptr,cnt,((cnt)+(num)),gran)

/*! \brief Allocate array (non-granular version) */
#define lh_array_allocate(ptr,cnt,num)  lh_array_allocate_g(ptr,cnt,num,1)

/*! \brief Resize array (non-granular version) */
#define lh_array_resize(ptr,cnt,num)    lh_array_resize_g(ptr,cnt,num,1)

/*! \brief Add elements to array (non-granular version) */
#define lh_array_add(ptr,cnt,num)       lh_array_add_g(ptr,cnt,num,1)


/*! \brief Delete a number of elements starting from a given position
 * Elements past the deleted range are moved to close the gap
 * The free space at the end is zeroed
 * The array is not resized
 * \param ptr Name of the pointer variable
 * \param cnt Name of the counter variable
 * \param from Index of the first element to delete
 * \param num Number of elements to delete
 */
#define lh_array_delete_range_nu(ptr, cnt, from, num) {                 \
        memmove((ptr)+(from), (ptr)+(from)+(num),                       \
                ((cnt)-(num)-(from))*sizeof(*ptr));                     \
        lh_clear_range(ptr,(cnt)-(num),num);                            \
    }                                                                   

/*! \brief Delete a single element at given position, but do not resize the array
 * \param ptr Name of the pointer variable
 * \param cnt Name of the counter variable
 * \param idx Index of the element to delete
 */
#define lh_array_delete_element_nu(ptr, cnt, idx)   \
    lh_array_delete_range_nu(ptr, cnt, idx, 1)

/*! \brief Delete range of elements and update cnt to resize the array
 * \param ptr Name of the pointer variable
 * \param cnt Name of the counter variable
 * \param from Index of the first element to delete
 * \param num Number of elements to delete
 */
#define lh_array_delete_range(ptr, cnt, from, num) {                    \
        lh_array_delete_range_nu(ptr, cnt, from, num);                  \
        cnt -= num;                                                     \
    }

/*! \brief Delete a single element and update cnt to resize the array
 * \param ptr Name of the pointer variable
 * \param cnt Name of the counter variable
 * \param idx Index of the element to delete
 */
#define lh_array_delete_element(ptr, cnt, idx)  \
    lh_array_delete_range(ptr, cnt, idx, 1)

////////////////////////////////////////////////////////////////////////////////

#ifdef LH_DECLARE_SHORT_NAMES

#define ALIGN(num,align)                lh_align(num,align)

#define CLEAR(x)                        lh_clear_obj(x)
#define CLEARP(x)                       lh_clear_ptr(x)
#define CLEARN(x,n)                     lh_clear_num(x,n)
#define CLEAR_RANGE(x,f,n)              lh_clear_range(x,f,n)

#define CREATE(t,n)                     lh_create_obj(t,n)
#define CREATEN(t,n,s)                  lh_create_num(t,n,s)
#define CREATEB(n,s)                    lh_create_buf(n,s)

#define ALLOC(p)                        lh_alloc_obj(p)
#define ALLOCN(p,n)                     lh_alloc_num(p,n)
#define ALLOCB(p,s)                     lh_alloc_buf(p,s)

#define RESIZE(p,n)                     lh_resize(p,n)
#define ARRAY(t,p,c)                    lh_array(t,p,c)
#define BUFFER(p,c)                     lh_buffer(p,c)

#define ARRAY_ALLOCG(p,c,n,g)           lh_array_allocate_g(p,c,n,g)
#define ARRAY_RESIZEG(p,c,n,g)          lh_array_resize_g(p,c,n,g)
#define ARRAY_ADDG(p,c,n,g)             lh_array_resize_g(p,c,n,g)

#define ARRAY_ALLOC(p,c,n)              lh_array_allocate(p,c,n)
#define ARRAY_RESIZE(p,c,n)             lh_array_resize(p,c,n)
#define ARRAY_ADD(p,c,n)                lh_array_resize(p,c,n)

#define ARRAY_DELETE_RANGE(p,c,f,n)     lh_array_delete_range(p,c,f,n)
#define ARRAY_DELETE(p,c,i)             lh_array_delete_element(p,c,i)
#define ARRAY_DELETE_RANGE_NU(p,c,f,n)  lh_array_delete_range_nu(p,c,f,n)
#define ARRAY_DELETE_NU(p,c,i)          lh_array_delete_element_nu(p,c,i)

#endif

