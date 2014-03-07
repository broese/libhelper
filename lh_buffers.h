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
 *
 * The resizable arrays are defined by two variables - a pointer variable \c ptr
 * and the integer counter variable \c cnt. The macros perform the allocation
 * and resizing of the arrays. Allocation granularity is defined in a separate
 * parameter.
 *
 * The use of the macros (reduction of the number of parameters) can be achieved
 * through special macros controlling default granularity and naming scheme.
 * Consider these examples:
 *
 * lh_array_allocate(ptr,cnt,4096,num)
 * - specify variables and parameters explicitly
 *
 * lh_array_allocate(AR(name),4096,num)
 * - use the standard naming scheme, implies variables name_ptr and name_cnt
 *
 * lh_array_allocate(GAR2(name),num)
 * - use the standard naming scheme and granularity 256
 *
 * lh_array_allocate(GAR(name),num)
 * - use the standard naming scheme and default granularity (LH_DEFAULT_GRAN)
 *
 * The user is able and is encouraged to define his own macros similar to AR
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

/*! \brief Resize the allocted memory of an array to a given number of elements.
 * \param ptr Pointer to allocated memory
 * \param num New number of elements to be available after allocation
 */
#define lh_resize(ptr, num) ptr = realloc(ptr, num*sizeof(*ptr));

/*! \brief Move elements in the allocated array. Does not check for boundaries
 * Does not zero elements. Source and destination ranges may overlap
 * \param ptr Pointer to allocated memory
 * \param from Index to move from
 * \param to Index to move to
Ü* \param num Number of elements to move
 */
#define lh_move(ptr, from, to, num) \
    memmove(ptr+to, ptr+from, num*sizeof(*ptr));

#ifndef LH_DEFAULT_GRAN
#define LH_DEFAULT_GRAN 256
#endif

#define AR(name) name##_ptr,name##_cnt

#define GAR(name) AR(name),LH_DEFAULT_GRAN

#define GAR0(name) AR(name),1
#define GAR1(name) AR(name),16
#define GAR2(name) AR(name),256
#define GAR3(name) AR(name),4096
#define GAR4(name) AR(name),65536
#define GAR5(name) AR(name),1048576

#define lh_arr_declare(type,name)   type * name ## _ptr; size_t name ## _cnt;
#define lh_arr_declare_i(type,name) type * name ## _ptr=NULL; size_t name ## _cnt=0;
#define lh_buf_declare(name)        lh_arr_declare(uint8_t,name)
#define lh_buf_declare_i(name)      lh_arr_declare_i(uint8_t,name)
#define lh_arr_init(ptr,cnt)        ptr=NULL, cnt=0;

#define _lh_arr_setgran(ptr,cnt,gran,clear) {                           \
        lh_resize((ptr),lh_align((cnt),(gran)))                         \
        if (clear) lh_clear_range(ptr,cnt,lh_align((cnt),(gran))-cnt);  \
    }

/*! \brief Ensure that the array allocation matches specific granularity
 * The array is resized if necessary and the remaining elements zeroed
 * \param ptr Name of the pointer variable
 * \param cnt Name of the counter variable
 * \param gran Granularity of allocation, must be power of 2
 */
#define lh_arr_setgran(...)   _lh_arr_setgran(__VA_ARGS__,0)
#define lh_arr_setgran_c(...) _lh_arr_setgran(__VA_ARGS__,1)

////////////////////////////////////////////////////////////////////////////////
// Allocate

#define _lh_arr_allocate(ptr,cnt,gran,num,clear) {         \
        lh_alloc_num((ptr), lh_align((num),(gran)));       \
        if (clear) lh_clear_num((ptr),(num));              \
        cnt = num;                                         \
    }

/*! \brief Allocate a resizable array to hold a given number of elements
 * \param ptr Pointer variable
 * \param cnt Counter variable
 * \param gran Allocation granularity
 * \param num Number of elements
 */
#define lh_arr_allocate(...)   _lh_arr_allocate(__VA_ARGS__,0)
#define lh_arr_allocate_c(...) _lh_arr_allocate(__VA_ARGS__,1)

////////////////////////////////////////////////////////////////////////////////
// Free

/*! \brief Free the memory allocated by a resizable array and reset the variables
 * \param ptr Pointer variable
 * \param cnt Counter variable
 */
#define _lh_arr_free(ptr,cnt,...)               \
    { if (ptr) free(ptr); ptr=NULL; cnt=0; }
#define lh_arr_free(...) _lh_arr_free(__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////

#define _lh_arr_resize(ptr,cnt,gran,num,clear) {                        \
        if (lh_align((num),(gran)) > lh_align((cnt),(gran))) {          \
            lh_resize((ptr),lh_align((num),(gran)));                    \
            if ( clear && (lh_align((num),gran)) > (cnt) )              \
                lh_clear_range((ptr), (cnt), (lh_align((num),gran))-(cnt)); \
        }                                                               \
        cnt=num;                                                        \
    }

/*! \brief Resize a resizable array to hold a given number of elements
 * \param ptr Pointer variable
 * \param cnt Counter variable
 * \param gran Allocation granularity
 * \param num New number of elements
 */
#define lh_arr_resize(...)   _lh_arr_resize(__VA_ARGS__,0)
#define lh_arr_resize_c(...) _lh_arr_resize(__VA_ARGS__,1)

////////////////////////////////////////////////////////////////////////////////

#define _lh_arr_add(ptr,cnt,gran,num)            \
    lh_arr_resize(ptr,cnt,((num)+(cnt)),gran)

#define _lh_arr_add_c(ptr,cnt,gran,num)          \
    lh_arr_resize_c(ptr,cnt,((num)+(cnt)),gran)

/*! \brief Add a number of elements to a resizable array
 * \param ptr Pointer variable
 * \param cnt Counter variable
 * \param gran Allocation granularity
 * \param num Number of elements to add
 */
#define lh_arr_add(...)   _lh_arr_add(__VA_ARGS__)
#define lh_arr_add_c(...) _lh_arr_add(__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////

#define _lh_arr_new(ptr,cnt,gran,clear) *({         \
            lh_arr_resize(ptr,cnt,gran,cnt+1);      \
            if (clear) lh_clear_ptr(ptr+cnt-1);     \
            ptr+cnt-1;                              \
        })

/*! \brief Resize the array to accomodate 1 new element at the end
 * This macro can be used as lvalue
 * \param ptr Pointer variable
 * \param cnt Counter variable
 * \param gran Allocation granularity
 * \return lvalue of the new element
 */
#define lh_arr_new(...)   _lh_arr_new(__VA_ARGS__,0)
#define lh_arr_new_c(...) _lh_arr_new(__VA_ARGS__,1)

////////////////////////////////////////////////////////////////////////////////

#define _lh_arr_insert(ptr,cnt,gran,idx,clear) *({      \
            lh_arr_resize(ptr,cnt,gran,cnt+1);          \
            lh_move(ptr,idx,idx+1,(cnt)-(idx));         \
            if (clear) lh_clear_ptr(ptr+idx);           \
            ptr+idx;                                    \
        })

/*! \brief Resize the array to accomodate 1 new element at the given position.
 * Elements after the position are moved to make space 
 * This macro can be used as lvalue
 * \param ptr Pointer variable
 * \param cnt Counter variable
 * \param gran Allocation granularity
 * \param idx Index at which the new element should be inserted
 * \return lvalue of the new element
 */
#define lh_arr_insert(...)   _lh_arr_insert(__VA_ARGS__,0)
#define lh_arr_insert_c(...) _lh_arr_insert(__VA_ARGS__,1)

////////////////////////////////////////////////////////////////////////////////

/*! \brief Delete a number of elements starting from a given position
 * Elements past the deleted range are moved to close the gap.
 * The array is resized but not reallocated
 * Note that this macro does not have the granularity parameter
 * Note that there is no variant that clears the tailing elements
 * \param ptr Pointer variable
 * \param cnt Counter variable
 * \param from First element index to delete
 * \param num Number of elements to delete
 */
#define _lh_arr_delete_range(ptr,cnt,from,num) {     \
        lh_move(ptr,from+num,from,cnt-num-from);    \
        cnt -= num;                                 \
    }
#define lh_arr_delete_range(...) _lh_arr_delete_range(__VA_ARGS__)

/*! \brief Delete one element at given position
 * Elements past the deleted one are moved to close the gap
 * The array is resized but not reallocated
 * Note that this macro does not have the granularity parameter
 * Note that there is no variant that clears the tailing elements
 * \param ptr Pointer variable
 * \param cnt Counter variable
 * \param idx Element index to delete
 */
#define _lh_arr_delete(ptr,cnt,idx)                  \
    lh_arr_delete_range(ptr,cnt,idx,1)
#define lh_arr_delete(...) _lh_arr_delete(__VA_ARGS__)

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
 *
 * The user is encouraged to create a macro that combines all fields, e.g.
 * #define MAF_FOO MAF(ptr1),MAF(ptr2),MAF(ptr3)
 * lh_multiarray_allocate(cnt, num, MAF_FOO);
 *
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
        lh_move(*ptrp, so*(from+num), so*from, so*(*cnt-num-from));
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

#ifndef LH_BUFPRINTF_GRAN
#define LH_BUFPRINTF_GRAN 256
#endif

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
        lh_arr_setgran(*bufp, *lenp, gran);
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

#define _lh_bufprintf(ptr,cnt,fmt,...)                                  \
    lh_bufprintf_g(&ptr,&cnt,LH_BUFPRINTF_GRAN,fmt,##__VA_ARGS__)
#define lh_bufprintf(...) _lh_bufprintf(__VA_ARGS__)

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
#define MOVE                            lh_move
#define setgran                         lh_arr_setgran
#define setgran_c                       lh_arr_setgran_c

#define _AR                             lh_arr_declare
#define _ARi                            lh_arr_declare_i
#define _BUF                            lh_buf_declare
#define _BUFi                           lh_buf_declare_i
#define arr_init                        lh_arr_init
#define arr_free                        lh_arr_free

#define arr_alloc                       lh_arr_allocate
#define arr_alloc_c                     lh_arr_allocate_c
#define arr_resize                      lh_arr_resize
#define arr_resize_c                    lh_arr_resize_c
#define arr_add                         lh_arr_add
#define arr_add_c                       lh_arr_add_c
#define arr_new                         lh_arr_new
#define arr_new_c                       lh_arr_new_c
#define arr_insert                      lh_arr_insert
#define arr_insert_c                    lh_arr_insert_c
#define arr_delrange                    lh_arr_delete_range
#define arr_delete                      lh_arr_delete

#define marr_alloc                      lh_multiarray_allocate
#define marr_alloc_g                    lh_multiarray_allocate_g
#define marr_resize                     lh_multiarray_resize
#define marr_resize_g                   lh_multiarray_resize_g
#define marr_add                        lh_multiarray_add
#define marr_add_g                      lh_multiarray_add_g
#define marr_delrange                   lh_multiarray_delete_range
#define marr_delete                     lh_multiarray_delete

#define bprintf                         lh_bufprintf

#endif
