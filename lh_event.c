#include "lh_event.h"
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>

int lh_poll_add(lh_pollarray *pa, int fd, short mode, int group, void *priv) {
    assert(pa);
    assert(fd>=0);
    lh_multiarray_add(pa->nfd,1,MAF(pa->poll),MAF(pa->data));

    struct pollfd *pf = pa->poll + pa->nfd-1;
    pf->fd = fd;
    pf->events = mode;
    pf->revents = 0;

    lh_polldata *pd = pa->data + pa->nfd-1;
    pd->group = group;
    pd->fd    = fd;
    pd->state = 0;
    pd->priv  = priv;

    return pa->nfd-1;    
}

int lh_poll_find(lh_pollarray *pa, int fd) {
    assert(pa);

    int i;
    for(i=0; i<pa->nfd; i++)
        if (pa->data[i].fd == fd)
            return i;
    return -1;
}

void lh_poll_remove(lh_pollarray *pa, int fd) {
    assert(pa);
    int i = lh_poll_find(pa,fd);
    assert(i>0);

    lh_multiarray_delete(pa->nfd, i, MAF(pa->poll),MAF(pa->data));
}

short *lh_poll_mode(lh_pollarray *pa, int fd) {
    assert(pa);
    int i = lh_poll_find(pa,fd);
    assert(i>0);

    return &pa->poll[i].events;
}

int lh_poll(lh_pollarray *pa, int timeout) {
    int res = poll(pa->poll, pa->nfd, timeout);
    if (res<0) LH_ERROR(-1, "poll() failed\n");

    int i;
    for(i=0; i<pa->nfd; i++)
        pa->data[i].state = pa->poll[i].revents;

    return res;
}

lh_polldata * lh_poll_getnext(lh_pollarray *pa, int *pos, int group, short mode) {
    assert(pa);

    for(;*pos<pa->nfd; (*pos)++) {
        lh_polldata *pd = pa->data + *pos;
        if (pd->group == group && (pd->state&mode)) {
            (*pos)++;
            return pd;
        }            
    }
    return NULL;
}

int lh_poll_getall(lh_pollarray *pa, int group, short mode, lh_polldata **pdp) {
    assert(pa);
    assert(pdp);

    int i,j,count;
    for(i=0;i<pa->nfd;i++) {
        lh_polldata *pd = pa->data+i;
        if (pd->group == group && (pd->state&mode))
            count++;
    }
    lh_alloc_num(*pdp,count);

    for(i=0,j=0;i<pa->nfd;i++) {
        lh_polldata *pd = pa->data+i;
        if (pd->group == group && (pd->state&mode))
            memcpy(*pdp+j++,pd,sizeof(lh_polldata));
    }
    return count;
}

////////////////////////////////////////////////////////////////////////////////

lh_conn * lh_conn_add(lh_pollarray *pa, int fd, int group, void *priv) {
    assert(pa);
    assert(fd>=0);

    lh_create_obj(lh_conn,conn);
    conn->pa = pa;
    conn->fd = fd;
    conn->status = 0;
    conn->priv = priv;

    lh_poll_add(pa, fd, POLLIN, group, conn);

    return conn;
}

void * lh_conn_remove(lh_conn *conn) {
    int i = lh_poll_find(conn->pa, conn->fd);
    if (i<0) return NULL;

    // delete everything in the lh_conn structure
    if (conn->rbuf.data_ptr) free(conn->rbuf.data_ptr);
    if (conn->wbuf.data_ptr) free(conn->wbuf.data_ptr);
    void * priv = conn->priv;
    free(conn);

    // remove the file descriptor from polling
    lh_poll_remove(conn->pa, conn->fd);

    // return the private data in case user needs it
    return priv;
}

void lh_conn_write(lh_conn *conn, uint8_t *data, ssize_t length) {
    assert(conn);
    assert(data);
    assert(!(conn->status&CONN_STATUS_LOCAL_EOF));

    if (!conn->wbuf.data_gran) // granularity was not specified - use default
        conn->wbuf.data_gran = LH_BUF_DEFAULT_GRAN;

    // copy data into write buffer, allocating as necessary
    ssize_t ridx = conn->wbuf.data_cnt;
    lh_arr_add(AR(conn->wbuf.data),conn->wbuf.data_gran, length);
    memmove(conn->wbuf.data_ptr+ridx, data, length);
    
    // try to send as much as possible
    ssize_t result = lh_write_buf(conn->fd, &conn->wbuf );

    //FIXME: handle errors

    // if not all data could be sent, set POLLOUT, so this buffer can
    // be transmitted asynchronously
    ssize_t remaining = conn->wbuf.data_cnt-conn->wbuf.data_ridx;
    assert(remaining>=0);
    if (remaining>0)
        lh_poll_w_on(conn->pa, conn->fd);
    else
        lh_poll_w_off(conn->pa, conn->fd);
}

void lh_conn_write_eof(lh_conn *conn) {
    assert(conn);
    conn->status |= CONN_STATUS_LOCAL_EOF;
    lh_poll_w_on(conn->pa, conn->fd);
}

#define COMPACT_THRESHOLD 256

void lh_conn_process(lh_pollarray *pa, int group, lh_conn_handler handler) {
    assert(pa);
    int pos=0;

    lh_polldata *pd;
    while (pd=lh_poll_getnext(pa, &pos, group, POLLIN)) {
        lh_conn *conn = (lh_conn *)conn->priv;
        ssize_t rbytes = lh_read_buf(conn->fd, &conn->rbuf);
       
        switch (rbytes) {
            case LH_FILE_INVALID:
            case LH_FILE_ERROR:
                conn->status = CONN_STATUS_ERROR;
                //FIXME: handle errors
                break;
            case LH_FILE_EOF:
                lh_poll_r_off(conn->pa, conn->fd);
                conn->status |= CONN_STATUS_REMOTE_EOF;
                //TODO: pass EOF to the handler?
                break;
            default: {
                ssize_t consumed = handler(conn);
                conn->rbuf.data_ridx += consumed;
                ssize_t remaining = conn->rbuf.data_cnt - conn->rbuf.data_ridx;
                if (remaining == 0) {
                    conn->rbuf.data_cnt = conn->rbuf.data_ridx = 0;
                }
                else if (remaining < COMPACT_THRESHOLD) {
                    lh_move(conn->rbuf.data_ptr, conn->rbuf.data_ridx, 0, remaining);
                    conn->rbuf.data_cnt = remaining;
                    conn->rbuf.data_ridx = 0;
                }
            }
        }
    }

    while (pd=lh_poll_getnext(pa, &pos, group, POLLOUT)) {
        lh_conn *conn = (lh_conn *)conn->priv;
        ssize_t wbytes = lh_write_buf(conn->fd, &conn->wbuf);
        switch (wbytes) {
            case LH_FILE_INVALID:
            case LH_FILE_ERROR:
                conn->status = CONN_STATUS_ERROR;
                //FIXME: handle errors
                break;
            default: {
                ssize_t remaining = conn->wbuf.data_cnt - conn->wbuf.data_ridx;
                if (remaining == 0) {
                    lh_poll_w_off(pa, conn->fd);
                    if (conn->status & CONN_STATUS_LOCAL_EOF)
                        shutdown(conn->fd, SHUT_WR);
                }
            }
        }
    }        
}
