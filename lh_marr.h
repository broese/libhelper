#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include "lh_buffers.h"

/**
 * \file Resizable Multi-Arrays
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

#ifdef LH_DECLARE_SHORT_NAMES

#define marr_alloc                      lh_multiarray_allocate
#define marr_alloc_g                    lh_multiarray_allocate_g
#define marr_resize                     lh_multiarray_resize
#define marr_resize_g                   lh_multiarray_resize_g
#define marr_add                        lh_multiarray_add
#define marr_add_g                      lh_multiarray_add_g
#define marr_delrange                   lh_multiarray_delete_range
#define marr_delete                     lh_multiarray_delete

#endif
