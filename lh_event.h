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
ssize_t evfile_read(int fd, uint8_t ** data, ssize_t *len, ssize_t maxread);
