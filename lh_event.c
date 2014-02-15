#include "lh_event.h"
#include <unistd.h>

int lh_poll_add(lh_pollarray * pa, lh_pollgroup *pg, int fd, short mode, void *data) {
    if (!pa) LH_ERROR(-1,"pollarray undefined");

    lh_multiarray_add_g(pa->nfd,1,LH_PA_GRAN,MAF(pa->poll),MAF(pa->data));

    lh_polldata *pd = pa->data + pa->nfd-1;
    struct pollfd *pf = pa->poll + pa->nfd-1;

    pd->fp          = NULL;
    pd->data        = data;
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

int lh_poll_add_fp(lh_pollarray * pa, lh_pollgroup *pg, FILE *fp, short mode, void *data) {
    int index = lh_poll_add(pa, pg, fileno(fp), mode, data);
    pa->data[index].fp = fp;
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
    if (datap) *datap   = pa->data[i].data;

    return pa->poll[i].fd;
}

int lh_poll_next_writable(lh_pollgroup *pg, FILE **fpp, void **datap) {
    lh_pollarray *pa = pg->pa;
    
    if (pg->wn <= 0) return -1;
    int i = pg->r[--pg->wn]; // take the last FD from the list and remove

    if (fpp) *fpp       = pa->data[i].fp;
    if (datap) *datap   = pa->data[i].data;

    return pa->poll[i].fd;
}

int lh_poll_next_error(lh_pollgroup *pg, FILE **fpp, void **datap) {
    lh_pollarray *pa = pg->pa;
    
    if (pg->en <= 0) return -1;
    int i = pg->r[--pg->en]; // take the last FD from the list and remove

    if (fpp) *fpp       = pa->data[i].fp;
    if (datap) *datap   = pa->data[i].data;

    return pa->poll[i].fd;
}

////////////////////////////////////////////////////////////////////////////////
// Layer 2

int lh_poll_read_once(int fd, uint8_t *ptr, ssize_t bufsize, ssize_t *len) {
    // how much we still can read to fill the buffer?
    ssize_t rlen = bufsize - *len;

    // how many bytes did we read actually?
    ssize_t rbytes = read(fd, ptr+(*len), rlen);

    if (rbytes < 0) { // error occured
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return LH_EVSTATUS_WAIT; // no more data, but no error
        else
            LH_ERROR(LH_EVSTATUS_ERROR, "Failed to read from fd=%d",fd);
    }

    if (rbytes == 0) { // no error, but end of file
        return LH_EVSTATUS_EOF;
    }

    *len += rbytes;

    if (rbytes < rlen) { // not enough data to read?
        return LH_EVSTATUS_WAIT;
    }

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
        boundary = lh_align(*lenp,LH_POLL_BUFGRAN);
        if (boundary == *lenp)
            boundary += LH_POLL_BUFGRAN;
    }
    else
        boundary = *lenp + maxread;

    LH_DEBUG("len=%zd maxread=%zd\n",*lenp,maxread);

    int res = LH_EVSTATUS_OK;
    do {
        // if we don't have anymore place in the buffer, allocate more
        if (bufsize - *lenp <= 0) {
            bufsize += LH_POLL_BUFGRAN;
            lh_resize(*ptrp, bufsize);
        }

        // determine up to which boundary should we read in this cycle
        ssize_t rbound = bufsize;
        if (rbound > boundary) rbound = boundary;

        res = lh_poll_read_once(fd, *ptrp, rbound, lenp);
    } while (res == LH_EVSTATUS_OK && *lenp < boundary );

    return res;
}

int lh_poll_write_once(int fd, uint8_t *ptr, ssize_t *lenp) {
    int res = LH_EVSTATUS_OK;

    ssize_t wbytes = write(fd, ptr, *lenp);
    if (wbytes < 0) {
        // error occured, no data written
        if (errno==EAGAIN || errno==EWOULDBLOCK)
            return LH_EVSTATUS_WAIT;
        else
            return LH_EVSTATUS_ERROR;
    }

    if (wbytes == *lenp) {
        // successful write, no issues
        *lenp = 0;
    }
    else if (wbytes < *lenp) {
        // partial write, no error, but a next write may return one
        // remove the written chunk of data from the buffer
        lh_array_delete_range(ptr, *lenp, 0, wbytes);
    }
    else {
        LH_ERROR(LH_EVSTATUS_ERROR,"wbytes > len");
    }
    return LH_EVSTATUS_OK;
}
