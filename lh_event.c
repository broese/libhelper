#include "lh_event.h"

// add a new file descriptor (int) to the pollarray
int pollarray_add(pollarray * pa, pollgroup * pg, int fd, short mode, void * data) {
    int i = pa->num;

    // add the new fd and associated elements at the end of list
    ARRAY_ADD(FILE *,pa->files,pa->num,1); pa->num=i; pa->files[i] = NULL;
    ARRAY_ADD(struct pollfd,pa->p,pa->num,1); pa->num=i;
    pa->p[i].fd = fd; pa->p[i].events = mode; pa->p[i].revents = 0;
    ARRAY_ADD(void *,pa->data,pa->num,1); pa->num=i; pa->data[i] = data;
    ARRAY_ADD(pollgroup *,pa->group,pa->num,1); pa->group[i] = pg;

    // resize arrays in the associated pollgroup
    int j = pg->num;
    ARRAY_ADD(int, pg->rfds, pg->num, 1); pg->num=j;
    ARRAY_ADD(int, pg->wfds, pg->num, 1); pg->num=j;
    ARRAY_ADD(FILE *, pg->rfiles, pg->num, 1); pg->num=j;
    ARRAY_ADD(FILE *, pg->wfiles, pg->num, 1); pg->num=j;
    ARRAY_ADD(void *, pg->rdata, pg->num, 1); pg->num=j;
    ARRAY_ADD(void *, pg->wdata, pg->num, 1);

    return i;
}

// wrapper function to add a FILE* instead of integer fd
int pollarray_add_file(pollarray * pa, pollgroup * pg, FILE *fd, short mode, void * data) {
    int i = pollarray_add(pa, pg, fileno(fd), mode, data);
    pa->files[i] = fd;
    return i;
}

// remove the 
int pollarray_remove(pollarray * pa, int fd) {
    int i;
    for(i=0; i<pa->num; i++)
        if (pa->p[i].fd == fd) {
            pollgroup * pg = pa->group[i];
            if (i<pa->num-1) {
                // this element is not at the end of list, need to move other elements
                ARRAY_DELETE(struct pollfd, pa->p, pa->num, i);
                ARRAY_DELETE(FILE *, pa->files, pa->num, i);
                ARRAY_DELETE(void *, pa->data, pa->num, i);
                ARRAY_DELETE(pollgroup *, pa->group, pa->num, i);
            }
            pa->num--;

            //TODO: resize the pollgroup properly - although it makes little sense
            pg->num--;
            pg->rnum=0; pg->wnum=0; // invalidate array contents in the pollgroup

            return i;
        }

    return -1;
}

int pollarray_remove_file(pollarray *pa, FILE *fd) {
    return pollarray_remove(pa, fileno(fd));
}

int evfile_poll(pollarray *pa, int timeout) {
    if (timeout<0) timeout = DEFAULT_POLL_TIMEOUT;

    int sel = poll(pa->p, pa->num, timeout);
    if (sel<0) LH_ERROR(-1, "poll on %d descriptors failed", pa->num);

    int i;

    // clear all groups
    //FIXME: optimize?
    for (i=0; i<pa->num; i++) {
        pa->group[i]->rnum = 0;
        pa->group[i]->wnum = 0;
    }

    // distribute events
    for (i=0; i<pa->num; i++) {
        pollgroup * pg = pa->group[i];
        if (pa->p[i].revents & POLLIN) {
            pg->rfds[pg->rnum]   = pa->p[i].fd;
            pg->rfiles[pg->rnum] = pa->files[i];
            pg->rdata[pg->rnum]  = pa->data[i];
            pg->rnum++;
        }
        if (pa->p[i].revents & POLLOUT) {
            pg->wfds[pg->wnum]   = pa->p[i].fd;
            pg->wfiles[pg->wnum] = pa->files[i];
            pg->wdata[pg->wnum]  = pa->data[i];
            pg->wnum++;
        }
    }
    return 0;
}



#if 0
void efile_add_fd(struct pollfd *p, int *num, int fd, short mode) {
    int i = *num;
    ARRAY_ADD(struct pollfd, p, *num, 1);
    p[i].fd = fd;
    p[i].events = mode;
    p[i].revents = 0;
}

int efile_poll(struct pollfd *p, int num, int timeout) {
    if (timeout < 0) timeout=DEFAULT_POLL_TIMEOUT;
    int sel = poll(p, num, timeout);
    if (sel<0) LH_ERROR(-1, "poll on %d descriptors failed", 
}
#endif



#if 0
int poll_fds(const int *fds, int *efds, int nfds, int read, int timeout) {
    ALLOCN(struct pollfd, pfds, nhds);

    int i;
    for (i=0; i<nfds; i++) {
        pfds[i].fd = fds[i];
        pfds[i].events = read?POLLIN:POLLOUT; //FIXME: do we need POLLPRI, too?
        pfds[i].revents = 0;
    }

    int sfds = poll(pfds, nfds, timeout);

    if (sfds < 0) { free(pfds); LH_ERROR(-1, "poll on %d fds failed",nfds); }

    int ei = 0;
    for(i=0; i<nfds; i++)
        if (pfds[i].revents & POLLIN)
            efds[ei++] = fds[i];

    free(pfds);
    return ei;
}

int poll_files(const FILE **files, FILE **efds, int nfds, int read, int timeout) {
    ALLOCN(struct pollfd, pfds, nhds);

    int i;
    for (i=0; i<nfds; i++) {
        pfds[i].fd = fileno(files[i]);
        pfds[i].events = read?POLLIN:POLLOUT; //FIXME: do we need POLLPRI, too?
        pfds[i].revents = 0;
    }

    int sfds = poll(pfds, nfds, timeout);

    if (sfds < 0) { free(pfds); LH_ERROR(-1, "poll on %d fds failed",nfds); }

    int ei = 0;
    for(i=0; i<nfds; i++)
        if (pfds[i].revents & POLLIN)
            efds[ei++] = files[i];

    free(pfds);
    return ei;
}
#endif

////////////////////////////////////////////////////////////////////////////////

#if 0
void pollblock_add_fd(pollblock_fd *pb, int fd) {
    int num = pb->num;
    ARRAY_ADD(struct pollfd, pb->p, pb->num, 1); pb->num = num;
    ARRAY_ADD(int, pb->f, pb->num, 1); pb->num = num;
    ARRAY_ADD(int, pb->r, pb->num, 1); pb->num = num;
    ARRAY_ADD(int, pb->w, pb->num, 1);

    pb->p[num].fd = fd;
    pb->p[num].events = POLLIN|POLLOUT;  //FIXME: which events should be included?
    pb->p[num].revents = 0;

    pb->f[num] = fd;
    pb->nr = 0;
    pb->nw = 0;
}

void pollblock_add_file(pollblock_file *pb, FILE *fd) {
    int num = pb->num;
    ARRAY_ADD(struct pollfd, pb->p, pb->num, 1); pb->num = num;
    ARRAY_ADD(FILE *, pb->f, pb->num, 1); pb->num = num;
    ARRAY_ADD(FILE *, pb->r, pb->num, 1); pb->num = num;
    ARRAY_ADD(FILE *, pb->w, pb->num, 1);

    pb->p[num].fd = fileno(fd);
    pb->p[num].events = POLLIN|POLLOUT;  //FIXME: which events should be included?
    pb->p[num].revents = 0;

    pb->f[num] = fd;
    pb->nr = 0;
    pb->nw = 0;
}

//TODO: implement pollblock_remove

////////////////////////////////////////////////////////////////////////////////

int pollblock_poll_fd(pollblock_fd *pb, int timeout) {
    //FIXME: do we need to clear revents or reinit pollfd?

    if (timeout < 0) timeout = DEFAULT_POLL_TIMEOUT;
    int selected = poll(pb->p, pb->num, timeout);
    if (selected < 0) LH_ERROR(-1, "poll on %d fds failed",pb->num);

    int i;
    pb->nr = 0;
    pb->nw = 0;
    
    for (i=0; i<pb->num; i++) {
        if (pb->p[i].revents & POLLIN) pb->r[pb->nr++] = pb->f[i];
        if (pb->p[i].revents & POLLOUT) pb->w[pb->nw++] = pb->f[i];
    }

    return 0;
}

int pollblock_poll_file(pollblock_file *pb, int timeout) {
    //FIXME: do we need to clear revents or reinit pollfd?

    if (timeout < 0) timeout = DEFAULT_POLL_TIMEOUT;
    int selected = poll(pb->p, pb->num, timeout);
    if (selected < 0) LH_ERROR(-1, "poll on %d fds failed",pb->num);

    int i;
    pb->nr = 0;
    pb->nw = 0;
    
    for (i=0; i<pb->num; i++) {
        if (pb->p[i].revents & POLLIN) pb->r[pb->nr++] = pb->f[i];
        if (pb->p[i].revents & POLLOUT) pb->w[pb->nw++] = pb->f[i];
    }

    return 0;
}
#endif
