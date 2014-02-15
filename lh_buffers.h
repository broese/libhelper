/*! \file
 * Macros for handling buffers and arrays
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

////////////////////////////////////////////////////////////////////////////////
/// @name Generic Macros

/*! \brief Align a number to the nearest boundary
 * \param num Number of elements
 * \param align Alignment size
 * \return Aligned number
 */
#define lh_align(num,align) ((((num)-1)|(((__typeof__(num))align)-1)) + 1)

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

/*! \brief Ensure that the array allocation matches specific granularity
 * The array is resized if necessary and the remaining elements zeroed
 * \param ptr Name of the pointer variable
 * \param cnt Name of the counter variable
 * \param gran Granularity of allocation, must be power of 2
 */
#define lh_array_setgran(ptr, cnt, gran) {                              \
        lh_resize((ptr),lh_align((cnt),(gran)));                        \
        lh_clear_range((ptr), (cnt), (lh_align((cnt),gran))-(cnt));     \
    }

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

/**
 * @name Resizable Multi-Arrays
 * Resizable multi-arrays are similar to the normal resizable arrays, but here,
 * a single count variable defines size of multiple arrays, referenced by their
 * respective pointer variables.
 * The list of pointer variables is a variable argument list. Handling of this
 * list has certain limitations in the preprocessor. Instead of creating a set
 * of complex, limited and potentially incompatible macros, I went for a
 * compromise. Each element of the list should be enclosed with the macro MAF()
 * which provides the sizeof the elements to an inline function.
 *
 * EXAMPLE:
 * int cnt;
 * char **ptr1;
 * int *ptr2;
 * struct foo *ptr3;
 * lh_multiarray_allocate(cnt, num, MAF(ptr1), MAF(ptr2), MAF(ptr3));
 */

#define MAF(ptr) ((void **)(&(ptr))),sizeof(*ptr)

/*! \brief Allocate a multi-array.
 * This is an internal function used by the macros, do not use it directly.
 */
static inline void lh_multiarray_allocate_internal(int *cnt, int num, int gran, ...) {
    va_list fields;
    va_start( fields, gran );
    do {
        void **ptrp = va_arg(fields, void **);
        if (!ptrp) break;
        ssize_t so  = va_arg(fields, ssize_t);
        *ptrp = malloc((so)*lh_align(num,gran));
        memset(*ptrp, 0, (so)*lh_align(num,gran));
    } while (1);
    va_end( fields );
    *cnt = num;
}

/*! \brief Allocate a multi-array with granularity 1.
 * \param cnt Name of the counter variable.
 * \param num Number of elements to allocate
 * \param ... List of pointers to the array variables
 */
#define lh_multiarray_allocate(cnt,num,...)                         \
    lh_multiarray_allocate_internal(&cnt,num,1,__VA_ARGS__,NULL)

/*! \brief Allocate a multi-array with granularity 1.
 * \param cnt Name of the counter variable.
 * \param num Number of elements to allocate
 * \param gran Allocation granularity, must be power of 2
 * \param ... List of pointers to the array variables
 */
#define lh_multiarray_allocate_g(cnt,num,gran,...)                  \
    lh_multiarray_allocate_internal(&cnt,num,gran,__VA_ARGS__,NULL)



/*! \brief Resize a multi-array.
 * This is an internal function used by the macros, do not use it directly.
 */
static inline void lh_multiarray_resize_internal(int *cnt, int num, int gran, ...) {
    va_list fields;
    va_start( fields, gran );
    do {
        void **ptrp = va_arg(fields, void **);
        if (!ptrp) break;
        ssize_t so  = va_arg(fields, ssize_t);
        if ( lh_align(num,gran) > lh_align(*cnt,gran) ) {
            *ptrp = realloc(*ptrp, so*lh_align(num,gran));
            memset(((uint8_t*)*ptrp)+*cnt*so, 0, 
                   so*(lh_align(num,gran)-lh_align(*cnt,gran)));
        }
    } while (1);
    va_end( fields );
    *cnt = num;
}

/*! \brief Resize a multi-array.
 * \param cnt Name of the counter variable.
 * \param num Number of elements to allocate
 * \param ... List of pointers to the array variables
 */
#define lh_multiarray_resize(cnt,num,...)                               \
    lh_multiarray_resize_internal(&cnt,num,1,__VA_ARGS__,NULL)

/*! \brief Resize a multi-array with a specific granularity
 * \param cnt Name of the counter variable.
 * \param num Number of elements to allocate
 * \param gran Allocation granularity, must be power of 2
 * \param ... List of pointers to the array variables
 */
#define lh_multiarray_resize_g(cnt,num,gran,...)                        \
    lh_multiarray_resize_internal(&cnt,num,gran,__VA_ARGS__,NULL)

/*! \brief Add a number of elements to a multi-array
 * \param cnt Name of the counter variable.
 * \param num Number of elements to add
 * \param ... List of pointers to the array variables
 */
#define lh_multiarray_add(cnt,num,...)                                  \
    lh_multiarray_resize_internal(&cnt,num+cnt,1,__VA_ARGS__,NULL)

/*! \brief Add a number of elements to a multi-array, using granularity.
 * \param cnt Name of the counter variable.
 * \param num Number of elements to add
 * \param gran Allocation granularity, must be power of 2
 * \param ... List of pointers to the array variables
 */
#define lh_multiarray_add_g(cnt,num,gran,...)                           \
    lh_multiarray_resize_internal(&cnt,num+cnt,gran,__VA_ARGS__,NULL)


/*! \brief Delete a range of elements in a multi-array.
 * This is an internal function used by the macros, do not use it directly.
 */
static inline void lh_multiarray_delete_internal(int *cnt, int from, int num, ...) {
    va_list fields;
    va_start( fields, num );
    do {
        uint8_t **ptrp = va_arg(fields, uint8_t **);
        if (!ptrp) break;
        ssize_t so  = va_arg(fields, ssize_t);
        lh_array_delete_range_nu(*ptrp, *cnt*so, from*so, num*so);
    } while (1);
    va_end( fields );
    *cnt -= num;
}

/*! \brief Delete a range of elements in a multi-array.
 * \param cnt Name of the counter variable.
 * \param from Index of the first element to delete
 * \param num Number of elements to delete
 * \param ... List of pointers to the array variables
 */
#define lh_multiarray_delete_range(cnt,from,num,...)                \
    lh_multiarray_delete_internal(&cnt,from,num,__VA_ARGS__,NULL)

/*! \brief Delete a single element in a multi-array.
 * \param cnt Name of the counter variable.
 * \param idx Index of the element to delete
 * \param ... List of pointers to the array variables
 */
#define lh_multiarray_delete(cnt,idx,...)                       \
    lh_multiarray_delete_internal(&cnt,idx,1,__VA_ARGS__,NULL)

////////////////////////////////////////////////////////////////////////////////

#define LH_BUFPRINTF_GRAN 64

static inline ssize_t lh_bufprintf_g(uint8_t **bufp, ssize_t *lenp, int gran, const char *fmt,...) {
    // remaining space in the allocated buffer
    ssize_t remsize = lh_align(*lenp,gran)-*lenp;

    // allocate buffer if it's not allocated at all
    // so the snprintf won't bail out due to a NULL pointer
    if (!*bufp || *lenp==0) {
        lh_resize(*bufp,gran);
        *lenp = 0;
        remsize = gran;
    }
    else {
        // otherwise ensure granularity
        lh_array_setgran(*bufp, *lenp, gran);
    }

    uint8_t *pos = *bufp+*lenp;

    va_list args;

    va_start(args,fmt);
    int plen = vsnprintf(pos, remsize, fmt, args);
    va_end(args);

    if (plen < 0) return plen; // error occured

    if (plen >= remsize) {
        // The space was not sufficient for the written string
        // (including terminator). Resize and repeat
        lh_resize(*bufp, lh_align(plen+1+*lenp, gran));
        pos = *bufp+*lenp;

        va_start(args,fmt);
        plen = vsnprintf(pos, plen+1, fmt, args);
        va_end(args);

        if (plen < 0) return plen; // error occured
    }

    *lenp += plen;
    return plen;
}



////////////////////////////////////////////////////////////////////////////////

#ifdef LH_DECLARE_SHORT_NAMES

#define ALIGN                           lh_align

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
#define ARRAY                           lh_array
#define BUFFER                          lh_buffer

#define ARRAY_ALLOCG                    lh_array_allocate_g
#define ARRAY_RESIZEG                   lh_array_resize_g
#define ARRAY_ADDG                      lh_array_resize_g

#define ARRAY_ALLOC                     lh_array_allocate
#define ARRAY_RESIZE                    lh_array_resize
#define ARRAY_ADD                       lh_array_resize

#define ARRAY_DELETE_RANGE              lh_array_delete_range
#define ARRAY_DELETE                    lh_array_delete_element
#define ARRAY_DELETE_RANGE_NU           lh_array_delete_range_nu
#define ARRAY_DELETE_NU                 lh_array_delete_element_nu

#define MARR_ALLOCG                     lh_multiarray_allocate_g
#define MARR_RESIZEG                    lh_multiarray_resize_g
#define MARR_ADDG                       lh_multiarray_add_g

#define MARR_ALLOC                      lh_multiarray_allocate
#define MARR_RESIZE                     lh_multiarray_resize
#define MARR_ADD                        lh_multiarray_add

#define MARR_DELETE                     lh_multiarray_delete
#define MARR_DELETE_RANGE               lh_multiarray_delete_range

#define bprintf(buf,len,fmt,...)        lh_bufprintf_g(&buf, &len, LH_BUFPRINTF_GRAN, fmt, __VA_ARGS__)

#endif

