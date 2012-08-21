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
// evfile_poll() will update the arrays *fds,*files and *data with the
// list of currently readable and writable descriptors, respectively.
// these arrays are pre-allocated to the maximum number of monitored
// file descriptors in the group, this number is in 'num'
// 'files' array will contain FILE* if the user supplied it for
// the specific fd, otherwise NULL
// The user should pre-allocate and zero this structure before use
// The user can maintain multiple such structures and specify one of them
// when assigning a new file descriptor to a pollarray
typedef struct {
    int     num;        // allocated number

    int     rnum;       // number of readable descriptors
    int   * rfds;       // integer descriptors
    FILE ** rfiles;     // POSIX-style descriptors
    void ** rdata;      // optional opaque private data

    int     wnum;       // number of readable descriptors
    int   * wfds;       // integer descriptors
    FILE ** wfiles;     // POSIX-style descriptors
    void ** wdata;      // optional opaque private data
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
int evfile_poll(pollarray *pa, int timeout);

