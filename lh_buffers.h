#pragma once

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

// granularity macros
#define GRANREST(num,gran) ((gran)-((unsigned)num))%(gran)
#define GRANSIZE(num,gran) ((num)+GRANREST(num,gran))


// allocate array with 'num' elements of 'type'. Place the pointer to 'name' and number to 'nname'
#define ARRAY_ALLOCG(type,name,nname,num,gran)                          \
    ALLOCNE(type,name,GRANSIZE(num,gran));                              \
    nname = num;

// extend the allocated array 'name' with currently 'nname' elements to 'num'
#define ARRAY_EXTENDG(type,name,nname,num,gran)                         \
    if (GRANSIZE(num,gran) > GRANSIZE(nname,gran))                      \
        name = (type *)realloc(name,GRANSIZE(num,gran)*sizeof(type));   \
    nname = num;

// add a number of 
#define ARRAY_ADDG(type,name,nname,num,gran) ARRAY_EXTENDG(type,name,nname,nname+num,gran)


#define ARRAY_ALLOC(type,name,nname,num)     ARRAY_ALLOCG(type,name,nname,num,1)
#define ARRAY_EXTEND(type,name,nname,num)    ARRAY_EXTENDG(type,name,nname,num,1)
#define ARRAY_ADD(type,name,nname,num)       ARRAY_ADDG(type,name,nname,num,1)

#define BUFFER_ALLOCG(name,nname,size,gran)  ARRAY_ALLOCG(unsigned char,name,nname,size,gran)
#define BUFFER_EXTENDG(name,nname,size,gran) ARRAY_EXTENDG(unsigned char,name,nname,size,gran)
#define BUFFER_ADDG(name,nname,size,gran)    ARRAY_ADDG(unsigned char,name,nname,size,gran)

#define BUFFER_ALLOC(name,nname,size)        BUFFER_ALLOCG(name,nname,size,1)
#define BUFFER_EXTEND(name,nname,size,gran)  BUFFER_EXTENDG(name,nname,size,1)
#define BUFFER_ADD(name,nname,size,gran)     BUFFER_ADDG(name,nname,size,gran,1)
