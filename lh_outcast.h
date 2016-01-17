/*
 Authors:
 Copyright 2012-2015 by Eduard Broese <ed.broese@gmx.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version
 2 of the License, or (at your option) any later version.
*/

/* This is a dump for all the pieces from lh_buffers that we remove for now */

/*
determine the length of the __VA_ARGS__ list. This macro only works for up to 16
elements. This can be extended to up to 63 elements before you hit the
preprocessor own limit. This also does not work for the empty list - returns 1
*/

#define VA_LENGTH_16(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) N
#define VA_LENGTH(...) VA_LENGTH_16(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)

////////////////////////////////////////////////////////////////////////////////
// Expandable multi-arrays

/*
The concept is similar to an expandable array, but there are multiple
arrays of the same length. So, we have multiple pointers to data, with
potentially different sizeof-s of their elements, but with a commont
integer counter for the number of elements and same granularity.
The special macro is capable of simultaneously resizing a semi-arbitrary
list of pointer names. Technical limitation - their number must be between
1 and 8, inclusive.
TODO: resizing does not zero the new elements! Make it similar to ARRAY_EXTEND
*/

#define RESIZE_1(oldnum,num,ptr) RESIZE(ptr,num); CLEARN(ptr+oldnum,num-oldnum);
#define RESIZE_2(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_1(oldnum,num,__VA_ARGS__)
#define RESIZE_3(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_2(oldnum,num,__VA_ARGS__)
#define RESIZE_4(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_3(oldnum,num,__VA_ARGS__)
#define RESIZE_5(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_4(oldnum,num,__VA_ARGS__)
#define RESIZE_6(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_5(oldnum,num,__VA_ARGS__)
#define RESIZE_7(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_6(oldnum,num,__VA_ARGS__)
#define RESIZE_8(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_7(oldnum,num,__VA_ARGS__)
#define RESIZE_9(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_8(oldnum,num,__VA_ARGS__)
#define RESIZE_10(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_9(oldnum,num,__VA_ARGS__)
#define RESIZE_11(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_10(oldnum,num,__VA_ARGS__)
#define RESIZE_12(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_11(oldnum,num,__VA_ARGS__)
#define RESIZE_13(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_12(oldnum,num,__VA_ARGS__)
#define RESIZE_14(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_13(oldnum,num,__VA_ARGS__)
#define RESIZE_15(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_14(oldnum,num,__VA_ARGS__)
#define RESIZE_16(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_15(oldnum,num,__VA_ARGS__)

#define RESIZE_N_(oldnum,num,N,...) RESIZE_ ## N(oldnum,num,__VA_ARGS__)
#define RESIZE_N(oldnum,num,N,...) RESIZE_N_(oldnum,num,N,__VA_ARGS__)

#define ARRAYS_EXTENDG(cnt,num,gran,...) do {                           \
        int ARRAYS_EXTEND_SIZE = GRANSIZE(num,gran);                    \
        if (ARRAYS_EXTEND_SIZE > GRANSIZE(cnt,gran)) {                  \
            RESIZE_N(cnt, ARRAYS_EXTEND_SIZE, VA_LENGTH(__VA_ARGS__) ,__VA_ARGS__);     \
        }                                                               \
        cnt = num;                                                      \
    } while(0);

#define ARRAYS_EXTEND(cnt,num,...)      ARRAYS_EXTENDG(cnt,num,1,__VA_ARGS__)
#define ARRAYS_ADDG(cnt,inc,gran,...)   ARRAYS_EXTENDG(cnt,cnt+inc,gran,__VA_ARGS__)
#define ARRAYS_ADD(cnt,inc,...)         ARRAYS_EXTENDG(cnt,cnt+inc,1,__VA_ARGS__)

//TODO: ARRAYS_DELETE

////////////////////////////////////////////////////////////////////////////////
// Sorting macros


#ifdef __linux

#define SORTF_(name,type,op)                                            \
    static int SORTF_##name                                             \
    (const void * a, const void * b, void * arg) {                      \
        size_t offset = ((size_t)arg) & 0xffffff;                       \
        size_t size   = (size_t)arg >> 24;                              \
        switch (size) {                                                 \
        case 1: {                                                       \
            type ## int8_t * A = (type ## int8_t *)(((char *)a)+offset); \
            type ## int8_t * B = (type ## int8_t *)(((char *)b)+offset); \
            return op ((*A < *B) ? -1 : (*B < *A));                     \
        }                                                               \
        case 2: {                                                       \
            type ## int16_t * A = (type ## int16_t *)(((char *)a)+offset); \
            type ## int16_t * B = (type ## int16_t *)(((char *)b)+offset); \
            return op ((*A < *B) ? -1 : (*B < *A));                     \
        }                                                               \
        case 4: {                                                       \
            type ## int32_t * A = (type ## int32_t *)(((char *)a)+offset); \
            type ## int32_t * B = (type ## int32_t *)(((char *)b)+offset); \
            return op ((*A < *B) ? -1 : (*B < *A));                     \
        }                                                               \
        case 8: {                                                       \
            type ## int64_t * A = (type ## int64_t *)(((char *)a)+offset); \
            type ## int64_t * B = (type ## int64_t *)(((char *)b)+offset); \
            return op ((*A < *B) ? -1 : (*B < *A));                     \
        }                                                               \
        default:                                                        \
            printf("Unsipported size %zd\n",size);                      \
        }                                                               \
    }

SORTF_(SI,,)
SORTF_(UI,u,)
SORTF_(SD,,-)
SORTF_(UD,u,-)


#if 0
static int SORTF_1S_INC(const void * a, const void * b, void * arg) {
    long offset = (long)arg;
    char * A = ((char *)a)+offset;
    char * B = ((char *)b)+offset;
    return (*A < *B) ? -1 : ( *A > *B);
}
#endif

#define SORT_(ptr, cnt, offset, size, suffix) qsort_r(ptr, cnt, sizeof(*ptr), SORTF_ ## suffix, (void *)((((size)&0xff)<<24)+((offset)&0xffffff)))
#define SORT(ptr, cnt, elem) SORT_(ptr, cnt, ((char *)&(elem)-(char *)(ptr)), sizeof(elem), SI)
#define SORTD(ptr, cnt, elem) SORT_(ptr, cnt, ((char *)&(elem)-(char *)(ptr)), sizeof(elem), SD)
#define SORTU(ptr, cnt, elem) SORT_(ptr, cnt, ((char *)&(elem)-(char *)(ptr)), sizeof(elem), UI)
#define SORTUD(ptr, cnt, elem) SORT_(ptr, cnt, ((char *)&(elem)-(char *)(ptr)), sizeof(elem), UD)

#else

#define SORT(ptr, cnt, elem) printf("qsort_r not available\n");

#endif


