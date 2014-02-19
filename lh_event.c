#include "lh_event.h"
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>

int lh_poll_add(lh_pollgroup *pg, int fd, short mode, void *priv) {
    assert(pg);
    lh_pollarray * pa = pg->pa;
    assert(pa);

    lh_multiarray_add_g(pa->nfd,1,LH_PA_GRAN,MAF(pa->poll),MAF(pa->data));

    lh_polldata *pd = pa->data + pa->nfd-1;
    struct pollfd *pf = pa->poll + pa->nfd-1;

    pd->fp          = NULL;
    pd->priv        = priv;
    pd->group       = pg;

    pf->fd          = fd;
    pf->events      = mode;
    pf->revents     = 0;

    if (pg) {
        lh_multiarray_add_g(pg->num,1,LH_PG_GRAN,MAF(pg->r),MAF(pg->w),MAF(pg->e));
        pg->pa = pa;
    }

    return pa->nfd-1;
}

int lh_poll_add_fp(lh_pollgroup *pg, FILE *fp, short mode, void *priv) {
    int index = lh_poll_add(pg, fileno(fp), mode, priv);
    pg->pa->data[index].fp = fp;
    return index;
}

int lh_poll_find(lh_pollarray *pa, int fd) {
    int i;
    for(i=0; i<pa->nfd; i++)
        if (pa->poll[i].fd == fd)
            return i;
    return -1;
}

int lh_poll_find_fp(lh_pollarray *pa, FILE *fp) {
    return lh_poll_find(pa, fileno(fp));
}

int lh_poll_remove(lh_pollarray *pa, int fd) {
    // locate the file descriptor in the list
    int i = lh_poll_find(pa, fd);
    if (i<0) return i;

    // invalidate the pollgroup, since indices may have changed
    lh_pollgroup *pg = pa->data[i].group;
    pg->rn = pg->wn = pg->en = 0;

    lh_multiarray_delete(pa->nfd, i, MAF(pa->poll), MAF(pa->data));

    return 0;
}

int lh_poll_remove_fp(lh_pollarray *pa, FILE *fp) {
    return lh_poll_remove(pa, fileno(fp));
}

void * lh_poll_priv(lh_pollarray *pa, int fd) {
    int i = lh_poll_find(pa, fd);
    if (i<0) return NULL;

    return pa->data[i].priv;
}

void * lh_poll_priv_fp(lh_pollarray *pa, FILE *fp) {
    return lh_poll_priv(pa, fileno(fp));
}

////////////////////////////////////////////////////////////////////////////////

int lh_poll(lh_pollarray *pa, int timeout) {
    int i;

    if (timeout<0) timeout = LH_POLL_TIMEOUT;

    // clear all pollgroups
    for(i=0; i<pa->nfd; i++) {
        lh_pollgroup *pg = pa->data[i].group;
        if (pg) pg->rn = pg->wn = pg->en = 0;
    }

    // poll the pollarray
    int sel = poll(pa->poll, pa->nfd, timeout);
    if (sel<0) LH_ERROR(-1, "poll on %d descriptors failed", pa->nfd);
    if (sel == 0) return 0; // no events

    // update the pollgroups
    for (i=0; i<pa->nfd; i++) {
        short revents = pa->poll[i].revents;
        lh_pollgroup *pg = pa->data[i].group;
        if (revents & POLLIN)                     pg->r[pg->rn++] = i;
        if (revents & POLLOUT)                    pg->w[pg->wn++] = i;
        if (revents & (POLLERR|POLLHUP|POLLNVAL)) pg->e[pg->en++] = i;
    }
    return 0;
}

int lh_poll_next_readable(lh_pollgroup *pg, FILE **fpp, void **datap) {
    lh_pollarray *pa = pg->pa;
    
    if (pg->rn <= 0) return -1;
    int i = pg->r[--pg->rn]; // take the last FD from the list and remove

    if (fpp) *fpp       = pa->data[i].fp;
    if (datap) *datap   = pa->data[i].priv;

    return pa->poll[i].fd;
}

int lh_poll_next_writable(lh_pollgroup *pg, FILE **fpp, void **datap) {
    lh_pollarray *pa = pg->pa;
    
    if (pg->wn <= 0) return -1;
    int i = pg->r[--pg->wn]; // take the last FD from the list and remove

    if (fpp) *fpp       = pa->data[i].fp;
    if (datap) *datap   = pa->data[i].priv;

    return pa->poll[i].fd;
}

int lh_poll_next_error(lh_pollgroup *pg, FILE **fpp, void **datap) {
    lh_pollarray *pa = pg->pa;
    
    if (pg->en <= 0) return -1;
    int i = pg->r[--pg->en]; // take the last FD from the list and remove

    if (fpp) *fpp       = pa->data[i].fp;
    if (datap) *datap   = pa->data[i].priv;

    return pa->poll[i].fd;
}

////////////////////////////////////////////////////////////////////////////////
// Layer 2

int lh_poll_read_once(int fd, uint8_t *ptr, ssize_t bufsize, ssize_t *len) {

    do {
        // how much we still can read to fill the buffer?
        ssize_t rlen = bufsize - *len;

        // consider the read successful if the buffer is already full
        //FIXME: should we return _OK, _ERROR or maybe _BUF?
        if (rlen == 0) return LH_EVSTATUS_OK;

        // how many bytes did we read actually?
        ssize_t rbytes = read(fd, ptr+(*len), rlen);

        if (rbytes < 0) { // error occured
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return LH_EVSTATUS_WAIT; // no error, just no more data to read
            else
                LH_ERROR(LH_EVSTATUS_ERROR, "Failed to read from fd=%d",fd);
        }

        if (rbytes == 0) { // no error, but end of file
            return LH_EVSTATUS_EOF;
        }

        assert(rbytes<=rlen);

        *len += rbytes;
    } while (*len < bufsize);
    // *len < bufsize means that read() returned less bytes than requested.
    // this is not an error, but may indicate an EAGAIN or EOF condition
    // that follows if we try to read again. Let's do that by repeating
    // the read and see if we hit one of these conditions.
    // It is also possible that we will read more data - e.g. if another
    // frame has arrived on a socket.

    // Otherwise, the buffer is full, return OK
    return LH_EVSTATUS_OK;
}

int lh_poll_read(int fd, uint8_t **ptrp, ssize_t *lenp, ssize_t maxread) {
    // ensure the buffer has proper granularity
    lh_array_setgran(*ptrp, *lenp, LH_POLL_BUFGRAN);
    
    // current allocated buffer size
    ssize_t bufsize = lh_align(*lenp,LH_POLL_BUFGRAN);

    // determine to which data length should we read at most
    ssize_t boundary;
    if (maxread <= 0) {
        // user did not specify maxread
        // by default, read to the next granularity boundary
        boundary = bufsize;

        // if buffer is already full, read to the next gran boundary 
        if (boundary == *lenp) boundary += LH_POLL_BUFGRAN;
    }
    else
        boundary = *lenp + maxread;

    int res = LH_EVSTATUS_OK;
    do {
        assert(*lenp<=bufsize);

        // if we don't have anymore place in the buffer, allocate more
        if (*lenp == bufsize) {
            bufsize += LH_POLL_BUFGRAN;
            lh_resize(*ptrp, bufsize);
        }

        // determine up to which boundary should we read in this cycle
        ssize_t rbound = bufsize;
        if (rbound > boundary) rbound = boundary;

        res = lh_poll_read_once(fd, *ptrp, rbound, lenp);
    } while (res == LH_EVSTATUS_OK && *lenp < boundary );

    // stop reading if there is an error, EOF, EAGAIN condition
    // or we have reached the read boundary

    return res;
}

int lh_poll_write_once(int fd, uint8_t *ptr, ssize_t *lenp) {
    do {
        ssize_t wbytes = write(fd, ptr, *lenp);
        
        if (wbytes < 0) {
            // error occured, no data written
            if (errno==EAGAIN || errno==EWOULDBLOCK)
                return LH_EVSTATUS_WAIT;
            else
                LH_ERROR(LH_EVSTATUS_ERROR,"Failed to write to fd=%d\n",fd);
        }

        assert(wbytes <= *lenp);

        if (wbytes == *lenp) {
            // all data written. Clear the buffer
            // the 'while' below will end this loop and return with OK
            *lenp = 0;
        }
        else {
            // at this point, less bytes were written than requested.
            // This is not an error, but may indicate an error (typically
            // EAGAIN) that will follow. Try to write once more to trigger this
            
            // remove the written chunk of data from the buffer
            lh_array_delete_range(ptr, *lenp, 0, wbytes);
        }
    } while(*lenp > 0);

    return LH_EVSTATUS_OK;
}

int lh_poll_read_buf(int fd, lh_buf *buf, ssize_t maxread) {
    return lh_poll_read(fd, &buf->ptr, &buf->len, maxread);
}

int lh_poll_write_buf(int fd, lh_buf *buf) {
    return lh_poll_write_once(fd, buf->ptr, &buf->len);
}

////////////////////////////////////////////////////////////////////////////////
// Layer 3

void lh_conn_add(lh_pollgroup *pg, int fd, void *priv) {
    lh_create_obj(lh_conn, conn);
    conn->fd = fd;
    conn->status = LH_EVSTATUS_OK;
    conn->priv = priv;
    lh_poll_add(pg, fd, MODE_R, conn);
}

void * lh_conn_remove(lh_pollgroup *pg, int fd) {
    lh_pollarray *pa = pg->pa;
    int i=lh_poll_find(pa, fd);
    if (i<0) return NULL;

    lh_conn * conn = (lh_conn *) pa->data[i].priv;
    if (conn->r.ptr) free(conn->r.ptr);
    if (conn->w.ptr) free(conn->w.ptr);

    void * priv = conn->priv;
    free(conn);

    lh_poll_remove(pa, fd);

    return priv;
}

void lh_conn_process(lh_pollgroup *pg, lh_handler handler) {
    int fd;
    lh_conn *conn;
    lh_pollarray *pa = pg->pa;

    // process incoming data
    while ((fd=lh_poll_next_readable(pg,NULL,(void **)&conn))>0) {
        int i = lh_poll_find(pg->pa, fd);
        conn->status = lh_poll_read_buf(fd, &conn->r, LH_BUF_MAXREAD);
        ssize_t hres = handler(conn);
        if (hres < 0) {
            // the handler does not wish to receive the data from this
            // connection anymore, possibly because of an error or EOF
            // We handle this by disabling the reading event (on EOF, poll()
            // would try to signal the readable condition all the time)
            // and shutting down the read direction
            pa->poll[i].events &= ~POLLIN;
            shutdown(fd, SHUT_RD);
            conn->r.len = 0;
        }
        else {
            // successful read, delete data consumed by the handler from
            // the rx buffer

            if (hres > 0) {
                assert(hres <= conn->r.len);
                lh_array_delete_range(conn->r.ptr, conn->r.len, 0, hres);
            }
        }

        // if there is data in the write buffer, trigger its transmission
        // by manipulating the POLLOUT flag
        if (conn->w.len > 0) {
            if (!(pa->poll[i].events & POLLOUT)) {
                pa->poll[i].events |= POLLOUT;
                pa->poll[i].revents |= POLLOUT;
            }
            // if the POLLOUT flag was already set, don't do anything -
            // either this fd was already flagged by poll() or if it's
            // not, it wasn't ready for transmission anyway.
        }
    }

    // process outgoing direction
    while ((fd=lh_poll_next_writable(pg,NULL,(void **)&conn))>0) {
        int i = lh_poll_find(pg->pa, fd);
        if (conn->w.len > 0) {
            int res = lh_poll_write_buf(fd, &conn->w);
            switch (res) {
            case LH_EVSTATUS_ERROR:
                // fd is no longer writable, stop checking for writable events
                pa->poll[i].events &= ~POLLOUT;
                shutdown(fd, SHUT_WR);
                // empty the wbuf, so we don't re-trigger writing condition
                conn->w.len = 0;
                break;
            case LH_EVSTATUS_WAIT:
                // could not write anymore data. Set the writable event,
                // so this data can be sent asynchronously later
                pa->poll[i].events |= POLLOUT;
                break;
            case LH_EVSTATUS_OK:
                // no more data to send, disable writable events for this fd
                pa->poll[i].events &= ~POLLOUT;
                break;
            }
        }
    }
}

int *lh_conn_cleanup(lh_pollgroup *pg) {
    lh_pollarray *pa = pg->pa;
    lh_array(int,fds,nfd);

    int i;
    for(i=0; i<pa->nfd; i++) {
        short events = pa->poll[i].events;
        if (!(events & (POLLIN|POLLOUT))) {
            lh_array_add(fds,nfd,1);
            fds[nfd-1] = pa->poll[i].fd;
        }
    }

    if (nfd > 0) {
        lh_array_add(fds,nfd,1);
        fds[nfd-1] = -1;
    }
    return fds;
}
