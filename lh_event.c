#include "lh_event.h"

////////////////////////////////////////////////////////////////////////////////
// Managing Pollarrays

#define DEFAULT_PA_GRAN 4

// add a new file descriptor (int) to the pollarray
int pollarray_add(pollarray * pa, pollgroup * pg, int fd, short mode, void * data) {
    int i = pa->num;

    ARRAYS_ADDG(pa->num,1,DEFAULT_PA_GRAN,pa->files,pa->p,pa->data,pa->group);
    
    pa->files[i] = NULL;
    pa->p[i].fd = fd; pa->p[i].events = mode; pa->p[i].revents = 0;
    pa->data[i] = data;
    pa->group[i] = pg;

    // resize arrays in the associated pollgroup
    int j = pg->num;

    ARRAYS_ADDG(pg->num,1,DEFAULT_PA_GRAN,pg->r,pg->w,pg->e);

    return i;
}

// wrapper function to add a FILE* instead of integer fd
int pollarray_add_file(pollarray * pa, pollgroup * pg, FILE *fd, short mode, void * data) {
    int i = pollarray_add(pa, pg, fileno(fd), mode, data);
    pa->files[i] = fd;
    return i;
}

int pollarray_find(pollarray * pa, int fd) {
    int i;
    for (i=0; i<pa->num; i++)
        if (pa->p[i].fd == fd)
            return i;
    return -1;
}

int pollarray_find_file(pollarray * pa, FILE *fd) {
    return pollarray_find(pa,fileno(fd));
}

// remove the file descriptor from the pollarray
int pollarray_remove(pollarray * pa, int fd) {
    int i = pollarray_find(pa,fd);
    if (i<0) return -1;

    // get the group before it is removed
    pollgroup * pg = pa->group[i];

    // delete the elements in all arrays at this index
    ARRAY_DELETE_NU(pa->p, pa->num, i);
    ARRAY_DELETE_NU(pa->files, pa->num, i);
    ARRAY_DELETE_NU(pa->data, pa->num, i);
    ARRAY_DELETE_NU(pa->group, pa->num, i);
    pa->num--;

    // resize the pollgroup
    pg->num--;

    // invalidate pollgroup
    pg->rn = pg->wn = pg->en = 0;

    return 0;
}

int pollarray_remove_file(pollarray *pa, FILE *fd) {
    return pollarray_remove(pa, fileno(fd));
}

////////////////////////////////////////////////////////////////////////////////

// poll all file descriptors in a poll array and update the associated poll groups
int evfile_poll(pollarray *pa, int timeout) {
    int i;

    if (timeout<0) timeout = DEFAULT_POLL_TIMEOUT;

    // clear all pollgroups
    //FIXME: optimize?
    for (i=0; i<pa->num; i++) {
        pollgroup *pg = pa->group[i];
        pg->rn = pg->wn = pg->en = 0;
    }

    // poll the entire pollarray
    int sel = poll(pa->p, pa->num, timeout);
    if (sel<0) LH_ERROR(-1, "poll on %d descriptors failed", pa->num);
    if (sel == 0) return 0;


    // distribute the events
    for (i=0; i<pa->num; i++) {
        pollgroup * pg = pa->group[i];
        if (pa->p[i].revents & POLLIN)                     pg->r[pg->rn++] = i;
        if (pa->p[i].revents & POLLOUT)                    pg->w[pg->wn++] = i;
        if (pa->p[i].revents & (POLLERR|POLLHUP|POLLNVAL)) pg->e[pg->en++] = i;
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

int evfile_read_once(int fd, uint8_t *ptr, ssize_t bufsize, ssize_t *len) {
    ssize_t rlen = bufsize - *len;
    ssize_t rbytes = read(fd, ptr+(*len), rlen);

    if (rbytes < 0) { // error occured
        int res = (errno == EAGAIN) ? EVFILE_WAIT : EVFILE_ERROR;
        LH_ERROR(res, "evfile_read_once: failed to read from fd=%d : %s\n",fd,strerror(errno));
    }

    if (rbytes == 0) { // no error, but end of file
        return EVFILE_EOF;
    }

    *len += rbytes;

    if (rbytes < rlen) { // not enough data to read?
        return EVFILE_WAIT;
    }

    return EVFILE_OK;
}

int evfile_read(int fd, uint8_t **ptr, ssize_t *len, ssize_t maxread) {
    // note - maxlen is aligned to BUFGRAN and so it may exceed the maxread
    // keep in mind that maxread is a soft limit - we will limit the reading
    // within a reasonable range of maxlen
    if (maxread<=0) maxread = EVFILE_BUFGRAN;
    int maxlen = *len + maxread;
    maxlen = GRANSIZE(maxlen, EVFILE_BUFGRAN);

    int res = EVFILE_OK;

    do {
        ssize_t nextgoal = GRANSIZE(*len, EVFILE_BUFGRAN) + EVFILE_BUFGRAN;
        if (nextgoal > maxlen) nextgoal = maxlen;

        ssize_t curlen = *len;
        ARRAY_EXTENDG(*ptr, curlen, nextgoal, EVFILE_BUFGRAN);
        res = evfile_read_once(fd, *ptr, curlen, len);
    } while (res == EVFILE_OK && *len < maxlen );

    return res;
}






#if 0
// read as much data into the buffer as possible, resizing it if needed
// int fd : file descriptor to read from
// uint8_t ** data : pointer to buffer pointer
// len : current size of data in the buffer, the data will be stored from this position
// maxread : maximum amount of data to read at once - it's a soft limit
// evfile_read uses it's own gran value, no need to arrange it with the user

#define EVFILE_READ_GRAN 65536

ssize_t evfile_read(int fd, uint8_t ** data, ssize_t *len, ssize_t maxread) {
    printf("%s:%d fd=%d data=%08x len=%d maxread=%d\n",__func__,__LINE__,fd,data,*len,maxread);

    // how many bytes to attempt to read next?
    // we try to read to the next granularity boundary, plus another granularity
    // block. I.e. if the buffer already contains 40000 bytes, we'll try
    // to read up to the 131072 bytes (128K) bjoundary, so nextread will be
    // 91072 bytes
    ssize_t nextread = GRANREST(*len, EVFILE_READ_GRAN) + EVFILE_READ_GRAN;
    printf("%s:%d len=%d nextread=%d\n",__func__,__LINE__,*len,nextread);

    ssize_t allocated = *len;
    ssize_t pos = *len;     // out current read position

    ssize_t totalread = 0; // total read bytes, for return value only

    printf("%s:%d\n",__func__,__LINE__);
    while (totalread < maxread) {
        // make room in the buffer
        ARRAY_ADDG(*data, allocated, nextread, EVFILE_READ_GRAN);
        ssize_t rbytes = read(fd, *data+pos, nextread);
        printf("rbytes=%d errno=%d %s\n",rbytes,errno,strerror(errno));

        if (rbytes < 0) break; // fatal error occured when reading
        totalread += rbytes;
        if (rbytes < nextread) break; // less than expected - EOF or wouldblock

        nextread = EVFILE_READ_GRAN;
    }

    *len += totalread;
    printf("%s:%d len=%d totalread=%d\n",__func__,__LINE__,len,totalread);
    return totalread;
}
#endif
