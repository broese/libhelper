#pragma once

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

typedef struct lh_polldata {
    int         group;
    int         fd;
    short       state;
    void *      priv;
} lh_polldata;

typedef struct {
    struct pollfd             * poll;
    struct lh_polldata        * data;
    int                         nfd;
} lh_pollarray;

int  lh_poll_add(lh_pollarray *pa, int fd, short mode, int group, void *priv);
void lh_poll_remove(lh_pollarray *pa, int fd);
int  lh_poll_find(lh_pollarray *pa, int fd);
short *lh_poll_mode(lh_pollarray *pa, int fd);

#define lh_poll_r_on(pa,fd)  *(lh_poll_mode(pa,fd)) |= POLLIN
#define lh_poll_r_off(pa,fd) *(lh_poll_mode(pa,fd)) &= ~POLLIN
#define lh_poll_w_on(pa,fd)  *(lh_poll_mode(pa,fd)) |= POLLOUT
#define lh_poll_w_off(pa,fd) *(lh_poll_mode(pa,fd)) &= ~POLLOUT

int lh_poll(lh_pollarray *pa, int timeout);

lh_polldata * lh_poll_getnext(lh_pollarray *pa, int *pos, int group, short mode);
lh_polldata * lh_poll_getfirst(lh_pollarray *pa, int group, short mode);
int lh_poll_getall(lh_pollarray *pa, int group, short mode, lh_polldata **pdp);
void lh_poll_dump(lh_pollarray *pa);

////////////////////////////////////////////////////////////////////////////////

typedef struct {
    lh_pollarray   *pa;
    int             fd;
    int             status;
    void           *priv;
    lh_buf_t        rbuf;
    lh_buf_t        wbuf;
} lh_conn;

#define CONN_STATUS_OK          0
#define CONN_STATUS_REMOTE_EOF  1
#define CONN_STATUS_LOCAL_EOF   2
#define CONN_STATUS_CLOSE       3
#define CONN_STATUS_ERROR       4

typedef ssize_t (*lh_conn_handler)(lh_conn *conn);

lh_conn * lh_conn_add(lh_pollarray *pa, int fd, int group, void *priv);
void * lh_conn_remove(lh_conn *conn);
void lh_conn_write(lh_conn *conn, uint8_t *data, ssize_t length);
void lh_conn_write_eof(lh_conn *conn);
void lh_conn_process(lh_pollarray *pa, int group, lh_conn_handler handler);

