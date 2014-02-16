#pragma once

#include "lh_debug.h"
#include "lh_buffers.h"
#include "lh_files.h"

#include <poll.h>

////////////////////////////////////////////////////////////////////////////////
/** @name Management of poll events
 * Low-level interface to poll() functions, management of registered file
 * descriptors
 */

#ifndef LH_POLL_TIMEOUT
#define LH_POLL_TIMEOUT 20
#endif

#define LH_PA_GRAN 16
#define LH_PG_GRAN 16

#define MODE_R  POLLIN
#define MODE_W  POLLOUT
#define MODE_RW (POLLIN|POLLOUT)

typedef struct lh_polldata lh_polldata;

/*! \brief This struct carries two arrays - \c poll and \c data. \c poll is
 * directly passed to poll(), while \c data carries all auxiliary data to the
 * file descriptors, since it cannot be stored in the <tt>struct pollfd</tt>.
 * Both arrays have the same length, stored in \c num
 */
typedef struct {
    int                 nfd;    // number of registered file descriptors
    struct pollfd     * poll;   // pollfd array - directly passed to poll()
    lh_polldata       * data;   // associated data
} lh_pollarray;

typedef struct {
    lh_pollarray *pa;   // associated poll array

    int num;        // number of allocated entries;

    int rn;         // number of readable descriptors
    int *r;         // readable descriptors (indices to pollarray)

    int wn;         // number of writable descriptors
    int *w;         // writable descriptors (indices to pollarray)

    int en;         // number of descriptors with errors
    int *e;         // erroneous descriptors (indices to pollarray)
} lh_pollgroup;
    
/*! \brief This struct carries additional data associated with a specific file
 * descriptor, since it cannot be stored in the standard struct pollfd
 */
typedef struct lh_polldata {
    FILE          * fp;     // file pointer associated with this file descriptor
                            // NULL if none was created
    void          * priv;   // opaque private data
    lh_pollgroup  * group;  // pointer to associated pollgroup
} lh_polldata;

////////////////////////////////////////////////////////////////////////////////

#define lh_pollarray_create(name)               \
    lh_pollarray name;                          \
    lh_clear_obj(name);

#define lh_pollgroup_create(name,paname)        \
    lh_pollgroup name;                          \
    lh_clear_obj(name);                         \
    name.pa = (paname);

int lh_poll_add(lh_pollgroup *pg, int fd, short mode, void *priv);
int lh_poll_add_fp(lh_pollgroup *pg, FILE *fp, short mode, void *priv);
int lh_poll_find(lh_pollarray *pa, int fd);
int lh_poll_find_fp(lh_pollarray *pa, FILE *fp);
int lh_poll_remove(lh_pollarray *pa, int fd);
int lh_poll_remove_fp(lh_pollarray *pa, FILE *fp);
void * lh_poll_priv(lh_pollarray *pa, int fd);
void * lh_poll_priv_fp(lh_pollarray *pa, FILE *fp);

int lh_poll(lh_pollarray *pa, int timeout);
int lh_poll_next_readable(lh_pollgroup *pg, FILE **fpp, void **privp);
int lh_poll_next_writable(lh_pollgroup *pg, FILE **fpp, void **privp);
int lh_poll_next_error(lh_pollgroup *pg, FILE **fpp, void **privp);

////////////////////////////////////////////////////////////////////////////////
/** @name Reading and writing using resizable buffers
 * Supporting functions to handle sending and receiving data via asynchronous
 * connections, typically network sockets.
 */

#define LH_POLL_BUFGRAN 4096

#define LH_EVSTATUS_OK    0
// successful read up to maxread and more data may be available

#define LH_EVSTATUS_WAIT  1
// no further data is available at the moment
// more data may arrive later, wait for another poll event

#define LH_EVSTATUS_EOF   2
// no errors, but encountered the end of file
// no more data will be available

#define LH_EVSTATUS_ERROR 3
// unsuccessful read, encountered an error and aborted the operation
// no more data will be available

/*! \brief Read data from a file descriptor, as much as possible, but
 * not exceeding the buffer size
 * \param fd File descriptor, preferable should be set to O_NONBLOCK mode,
 * otherwise the function will block until EOF, error or buffer is full
 * \param ptr Pointer to the start of buffer
 * \param bufsize Size of the buffer
 * \param len Pointer to a ssize_t variable indicating the current length
 * \return Status code - one of the LH_STATUS_* constants
 */
int lh_poll_read_once(int fd, uint8_t *ptr, ssize_t bufsize, ssize_t *len);

/*! \brief Read data from a file descriptor, as much as possible, but at most
 * the defined maxread amount, resizing buffer as necessary
 * \param fd File descriptor, preferable should be set to O_NONBLOCK mode,
 * otherwise the function will block until EOF, error or buffer is full
 * \param ptrp Pointer to the buffer pointer
 * \param lenp Pointer to the buffer length variable
 * \param maxread Maximum amount to read, set to <=0 to use default amount
 * (read to next LH_POLL_BUFGRAN boundary)
 * \return Status code - one of the LH_STATUS_* constants
 */
int lh_poll_read(int fd, uint8_t **ptrp, ssize_t *lenp, ssize_t maxread);

/*! \brief Write data to a file descriptor, as much as possible. Successfully
 * written data will be removed from the buffer.
 * \param fd File descriptor, preferable should be set to O_NONBLOCK mode,
 * otherwise the function will block file descriptor becomes writable again
 * \param ptr Pointer to the data buffer
 * \param lenp Pointer to the data length variable
 * \return Status code - one of the LH_STATUS_* constants
 */
int lh_poll_write_once(int fd, uint8_t *ptr, ssize_t *lenp);



#if 0


/*
  This is a retained piece of documentation from a previous version
  of event implementation. This info is still mostly valid, although
  the names have changed. The "Type 3" interface is not implemented

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

#endif

////////////////////////////////////////////////////////////////////////////////

#if 0

#ifndef LH_BUF_GRAN
#define LH_BUF_GRAN 4096
#endif

#ifndef LH_BUF_MAXREAD
#define LH_BUF_MAXREAD 65536
#endif

typedef struct {
    int fd;
    int status;
    uint8_t * rbuf;
    ssize_t   rlen;
    uint8_t * wbuf;
    ssize_t   wlen;
    void    * priv;
} lh_conn;

// return value : how much of the rx data has been consumed?
// 0..rlen : normal, -1 : close connection
typedef ssize_t (*lh_handler)(lh_conn *conn);

void lh_conn_add(lh_pollarray * pa, lh_pollgroup *pg, int fd, void * priv);
void lh_conn_process(lh_pollgroup *pg, lh_handler handler);

#endif
