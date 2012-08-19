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




#if 0





/*
The following structure defines a set of file descriptors that can be polled for file r/w events.
The structure is both the parameter (num,files,fds,data) and the result (xnum,xfiles,xfds,xdata)

*/

typedef {
#if 0
    struct pollfd * p;  // pollfd array this struct is bound to
    int   * pnum;       // + length var 
#endif

    short   mask;       // poll mask (e.g. POLLIN, POLLOUT or POLLIN|POLLOUT

    // list of file descriptors to monitor
    int     num;        // number of descriptors
    FILE ** files;      // list of FILE * - if none was supplied, contains NULLs
    int   * fds;        // 
    void  * data;

    // 
    int     snum;
    FILE ** sfiles;
    int   * sfds;
    void  * sdata;
} pollgroup;





void efile_add_fd(struct pollfd *p, int *num, int fd, short mode);
#define efile_add_file(p,num,file,mode) efile_add_fd(p,num,fileno(file),mode);

int efile_poll(struct pollfd *p, int num, int timeout);




#if 0
void efile_add_fd(struct pollfd *p, int *num, int fd, short mode);







////////////////////////////////////////////////////////////////////////////////

#if 0
int poll_fds(const int *fds, int *efds, int nfds, int read, int timeout);
#define poll_fds_read(fds, efds, nfds) poll_fds(fds, efds, nfds, 1, DEFAULT_POLL_TIMEOUT)
#define poll_fds_write(fds, efds, nfds) poll_fds(fds, efds, nfds, 0, DEFAULT_POLL_TIMEOUT)

int poll_files(const FILE **files, FILE **efds, int nfds, int read, int timeout);
#define poll_files_read(files, efds, nfds) poll_files(files, efds, nfds, 1, DEFAULT_POLL_TIMEOUT)
#define poll_files_write(files, efds, nfds) poll_files(files, efds, nfds, 0, DEFAULT_POLL_TIMEOUT)
#endif

////////////////////////////////////////////////////////////////////////////////

// WARNING: this structures must be zeroed after allocation in order to be
// consistently empty.
typedef struct {
    int num;
    struct pollfd * p;

    int * f;

    int * r;
    int nr;

    int * w;
    int nw;
} pollblock_fd;

typedef struct {
    int num;
    struct pollfd * p;

    FILE ** f;

    FILE ** r;
    int nr;

    FILE ** w;
    int nw;
} pollblock_file;

void pollblock_add_fd(pollblock_fd *pb, int fd);
void pollblock_add_file(pollblock_file *pb, FILE *fd);
int pollblock_poll_fd(pollblock_fd *pb, int timeout);
int pollblock_poll_file(pollblock_file *pb, int timeout);

#endif

#endif

