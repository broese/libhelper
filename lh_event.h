#pragma once

#include "lh_debug.h"
#include "lh_buffers.h"
#include "lh_files.h"

#include <poll.h>

#define DEFAULT_POLL_TIMEOUT 20
#define MODE_R  POLLIN
#define MODE_W  POLLOUT
#define MODE_RW (POLLIN|POLLOUT)

// struct pollgroup is used to return the results of the poll()
// evfile_poll() will fill the r,w and e arrays with the indexes of
// pollarray elements that became readable, writable, or have errors,
// respectively. The rn, wn and en integers store the number of indexes
// in each array.
// The r,w and e arrays must be pre-allocated to the total number of
// file descriptors managed by the group before calling evfile_poll(), in
// order to provide sufficient space for each situation.
// The user should pre-allocate and zero this structure before use
// The user can maintain multiple such structures and specify one of them
// when assigning a new file descriptor to a pollarray
typedef struct {
    int     num;        // allocated number

    int     rn;         // readable descriptors
    int   * r;

    int     wn;         // writable descriptors
    int   * w;

    int     en;         // descriptors with errors
    int   * e;
} pollgroup;

// semi-global array of descriptors polled in a single poll() calls
// the user can maintain multiple of such objects,
// though in most cases this won't be practical or necessary
typedef struct {
    int     num;        // total number of file descriptors
    FILE ** files;      // POSIX-style descriptors
    struct pollfd * p;  // pollfd array - contains integer descriptors and poll masks
    void ** data;       // opaque private data
    pollgroup ** group; // pointer to the group
} pollarray;

int pollarray_add(pollarray * pa, pollgroup * pg, int fd, short mode, void * data);
int pollarray_add_file(pollarray * pa, pollgroup * pg, FILE *fd, short mode, void * data);
int pollarray_remove(pollarray * pa, int fd);
int pollarray_remove_file(pollarray *pa, FILE *fd);
int pollarray_find(pollarray * pa, int fd);
int pollarray_find_file(pollarray * pa, FILE *fd);

int evfile_poll(pollarray *pa, int timeout);

////////////////////////////////////////////////////////////////////////////////
// read/write functions

/*
  Note on evfile-controlled reading and writing functions.
  As one of the arguments, they will expect a buffer, defined by:
   - uint8_t * ptr   - start of useful data
   - ssize     len   - length of useful data
   - ssize     alen  - total allocated size of the buffer,
                       which implies the size available for additional data

  This data can be conveyed to the function by three different means, represented
  by the three different interfaces to the function. Users may choose to use
  one of the interfaces depending if they want evfile to do the buffer
  management, or wish to keep control on it themselves. The functions that do
  the buffer and event management are merely wrappers to the base functions that
  perform actual reading and writing to file/socket.

  Type 1 - base function
  The base functions expect the full buffer description - ptr, len and alen.
  ptr is passed as a copy and will not be reallocated.
  len is passed per reference and may be updated by the read operation.
  alen is used by the read operation to know how much free space is available

  Base function is not capable of reading multiple times, it's limited to the
  available buffer space.

  Type 2 - buffer managing function
  This function receives a brief buffer description from the user - ptr and len
  This function assumes that the buffer was allocated with a specific granularity
  value - EVFILE_BUFGRAN and alen is derived from it.
  User can manage the buffer himself, but he must use the same granularity
  value. This function is the least safe of the three, use with caution.
  The function will make multiple calls to the base function, resizing the
  buffer as needed. In addition, user must supply the parameter maxlen, which
  defines the maximum amount of data to place in the buffer at most - this is
  needed to prevent reading too much data from a non-blocking source at once
  and allow the user to process it.

  Type 3 - evfile_buffer
  All buffer management is controlled by evfile functions only. User may
  control it, but must adhere to the data structure and values used by evfile.

  The functions receive not separate ptr, len and alen, but an evfile_buffer
  struct.

*/

#define EVFILE_BUFGRAN 4096

#define EVFILE_OK       0 // successful read up to maxread and more data may be available
#define EVFILE_WAIT     1 // successful read, but no further data available at the moment
                          // due to EAGAIN, more data could be available later
#define EVFILE_EOF      2 // successful read, but encountered end of file, so no more
                          // data will be available
#define EVFILE_ERROR    3 // unsuccessful read, encountered an error and aborted
                          // the operation - no more data will be available

int evfile_read_once(int fd, uint8_t *ptr, ssize_t bufsize, ssize_t *len);
int evfile_read(int fd, uint8_t **ptr, ssize_t *len, ssize_t maxread);

