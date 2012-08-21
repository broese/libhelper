#pragma once

#include <string.h>
#include <stdint.h>

/*
lh_buffers - macros for handling buffers and arrays
*/

////////////////////////////////////////////////////////////////////////////////

/* allocation macros

--------------------------------------------------------------------------------
Examples of allocation tasks for a new variable:
Macro: ALLOC, ALLOCN, ALLOCB

fixed-length buffer
char *buffer = (char *)malloc(SIZE);
memset(buffer, 0, SIZE)
ALLOCN(char, buffer, SIZE);
ALLOCB(buffer, SIZE);

fixed-size object
struct foo *baz = (struct *)malloc(sizeof(struct foo));
memset(baz, 0, sizeof(struct foo));
ALLOC(struct foo, baz);

array of objects
struct foo *baz = (struct *)malloc(sizeof(struct foo)*N);
memset(baz, 0, sizeof(struct foo)*N);
ALLOCN(struct foo, baz, N);

array of elements
int *baz = (struct *)malloc(sizeof(int)*N);
memset(baz, 0, sizeof(int)*N);
ALLOCN(int, baz, N);

--------------------------------------------------------------------------------
Examples of allocation tasks for an existing variable:
Macro: ALLOCE, ALLOCNE, ALLOCBE

fixed-length buffer
buffer = (char *)malloc(SIZE);
memset(buffer, 0, SIZE)

fixed-size object
baz = (struct *)malloc(sizeof(struct foo));
memset(baz, 0, sizeof(struct foo));

array of objects
baz = (struct *)malloc(sizeof(struct foo)*N);
memset(baz, 0, sizeof(struct foo)*N);

array of elements
baz = (struct *)malloc(sizeof(int)*N);
memset(baz, 0, sizeof(int)*N);

*/

// clear a single variable
#define CLEAR(name) memset(&name, 0, sizeof(name));

// These three macros assign the allocated element(s) to a new variable

// allocate single element of 'type'
#define ALLOC(type,name) ALLOCN(type,name,1)
// allocate a byte buffer of 'size' bytes
#define ALLOCB(name,size) ALLOCN(unsigned char,name,size)
// allocate 'num' elements of 'type'
#define ALLOCN(type,name,num) type * ALLOCNE(type,name,num)

// These three macros assign the allocated element(s) to an existing variable

// allocate single element of 'type'
#define ALLOCE(type,name) ALLOCNE(type,name,1)
// allocate a byte buffer of 'size' bytes
#define ALLOCBE(name,size) ALLOCNE(unsigned char,name,size)
// allocate 'num' elements of 'type'
#define ALLOCNE(type,name,num) name = (type *) malloc((num)*sizeof(type)); memset(name, 0, (num)*sizeof(type));

////////////////////////////////////////////////////////////////////////////////
// Expandable arrays and buffers
// they are defined through the 'type' of their elements, a type pointer 'name'
// and the array size variable 'nname'. The allocated size of the array can be 
// granular to 'gran' boundaries in order to re


// helper functions

// calculate the number of remaining elements from 'num' to the next 'gran' boundary
#define GRANREST(num,gran) ((gran)-((unsigned)num))%(gran)
// calculate the next 'gran' boundary after 'num'
#define GRANSIZE(num,gran) ((num)+GRANREST(num,gran))

// realloc() an array
//#define RESIZE_T(type, name, num) name = (type *)realloc(name, num*sizeof(type));


#define RESIZE(name, num) name = realloc(name, num*sizeof(*name));
#define RESIZE_SIZE(sname,nname,inc,gran) do {                          \
    int sname = GRANSIZE(nname+inc,gran);                               \
    if (sname > GRANSIZE(nname,gran) )

#define RESIZE_SIZE_FOOT(nname,inc)                                     \
    nname += inc;                                                       \
    } while(0);

#define RESIZE_1(name, nname, inc, gran)

// clear a number 'num' elements in the array
#define CLEAR_RANGE(type, name, from, num)                              \
    if ( num > 0 )                                                      \
        memset(name+from, 0, (num)*sizeof(type));

// delete a single element at position 'idx'
#define ARRAY_DELETE(type, name, nname, idx) memcpy((name)+(idx), (name)+(idx)+1, ((nname)-1-(idx))*sizeof(type));

// granular and non-granular allocation of arrays

// allocate array with 'num' elements of 'type'. Place the pointer to 'name' and number to 'nname'
#define ARRAY_ALLOCG(type,name,nname,num,gran)                          \
    ALLOCNE(type,name,GRANSIZE(num,gran));                              \
    nname = num;

// extend the allocated array 'name' with currently 'nname' elements to 'num'
#define ARRAY_EXTENDG(type,name,nname,num,gran) {                       \
    if (GRANSIZE(num,gran) > GRANSIZE(nname,gran))                      \
        RESIZE(name,GRANSIZE(num,gran));                                \
    CLEAR_RANGE(type, name, nname, GRANSIZE(num,gran)-nname);           \
    nname = num;                                                        \
    }

// add 'num' elements
#define ARRAY_ADDG(type,name,nname,num,gran) ARRAY_EXTENDG(type,name,nname,nname+num,gran)

#define ARRAY_ALLOC(type,name,nname,num)     ARRAY_ALLOCG(type,name,nname,num,1)
#define ARRAY_EXTEND(type,name,nname,num)    ARRAY_EXTENDG(type,name,nname,num,1)
#define ARRAY_ADD(type,name,nname,num)       ARRAY_ADDG(type,name,nname,num,1)

#define BUFFER_ALLOCG(name,nname,size,gran)  ARRAY_ALLOCG(unsigned char,name,nname,size,gran)
#define BUFFER_EXTENDG(name,nname,size,gran) ARRAY_EXTENDG(unsigned char,name,nname,size,gran)
#define BUFFER_ADDG(name,nname,size,gran)    ARRAY_ADDG(unsigned char,name,nname,size,gran)

#define BUFFER_ALLOC(name,nname,size)        BUFFER_ALLOCG(name,nname,size,1)
#define BUFFER_EXTEND(name,nname,size)       BUFFER_EXTENDG(name,nname,size,1)
#define BUFFER_ADD(name,nname,size)          BUFFER_ADDG(name,nname,size,1)




////////////////////////////////////////////////////////////////////////////////
// Extraction of values from a byte stream

#define PUTF *t++ = *p++
#define PUTR *t-- = *p++

static inline int8_t parse_char(const char *p) {
    return *p;
}

// Big-Endian extraction (aka network order)

static inline int16_t parse_short(const char *p) {
    union {
        char buf[2];
        int16_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[1];
    PUTR; PUTR;
#else
    unsigned char *t = &temp.buf[0];
    PUTF; PUTF;
#endif
    return temp.val;
}

static inline int32_t parse_int(const char *p) {
    union {
        char buf[4];
        int32_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[3];
    PUTR; PUTR;
    PUTR; PUTR;
#else
    unsigned char *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
#endif
    return temp.val;
}

static inline int64_t parse_long(const char *p) {
    union {
        char buf[8];
        int64_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[7];
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
#else
    unsigned char *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
#endif
    return temp.val;
}

#define read_char(p)  parse_char(p);  (p)++
#define read_short(p) parse_short(p); (p)+=2
#define read_int(p)   parse_int(p);   (p)+=4
#define read_long(p)  parse_long(p);  (p)+=8

// Little-Endian extraction (aka Intel order)

static inline int16_t parse_short_le(const char *p) {
    union {
        char buf[2];
        int16_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[0];
    PUTF; PUTF;
#else
    unsigned char *t = &temp.buf[1];
    PUTR; PUTR;
#endif
    return temp.val;
}

static inline int32_t parse_int_le(const char *p) {
    union {
        char buf[4];
        int32_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
#else
    unsigned char *t = &temp.buf[3];
    PUTR; PUTR;
    PUTR; PUTR;
#endif
    return temp.val;
}

static inline int64_t parse_long_le(const char *p) {
    union {
        char buf[8];
        int64_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
#else
    unsigned char *t = &temp.buf[7];
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
#endif
    return temp.val;
}

#define read_char_le(p)  parse_char(p);     (p)++
#define read_short_le(p) parse_short_le(p); (p)+=2
#define read_int_le(p)   parse_int_le(p);   (p)+=4
#define read_long_le(p)  parse_long_le(p);  (p)+=8

////////////////////////////////////////////////////////////////////////////////

static inline float parse_float(const char *p) {
    union {
        char buf[4];
        float val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[3];
    PUTR; PUTR;
    PUTR; PUTR;
#else
    unsigned char *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
#endif
    return temp.val;
}

static inline double parse_double(const char *p) {
    union {
        char buf[8];
        double val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[7];
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
#else
    unsigned char *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
#endif
    return temp.val;
}

#define read_float(p)   parse_float(p);   p+=4
#define read_double(p)  parse_double(p);  p+=8

////////////////////////////////////////////////////////////////////////////////

static inline float parse_float_le(const char *p) {
    union {
        char buf[4];
        float val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
#else
    unsigned char *t = &temp.buf[3];
    PUTR; PUTR;
    PUTR; PUTR;
#endif
    return temp.val;
}

static inline double parse_double_le(const char *p) {
    union {
        char buf[8];
        double val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
#else
    unsigned char *t = &temp.buf[7];
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
#endif
    return temp.val;
}

#define read_float_le(p)   parse_float_le(p);   p+=4
#define read_double_le(p)  parse_double_le(p);  p+=8

