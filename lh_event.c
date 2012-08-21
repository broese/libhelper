#include "lh_event.h"

#define DEFAULT_PA_GRAN 4

// add a new file descriptor (int) to the pollarray
int pollarray_add(pollarray * pa, pollgroup * pg, int fd, short mode, void * data) {
    int i = pa->num;

    printf("here: %s:%d\n",__func__,__LINE__);

    RESIZE_SIZE(size, pa->num, 1, DEFAULT_PA_GRAN) {
        printf("here: %s:%d size=%d\n",__func__,__LINE__,size);
        RESIZE(pa->files,size);
        RESIZE(pa->p,size);
        RESIZE(pa->data,size);
        RESIZE(pa->group,size);
    } RESIZE_SIZE_FOOT(pa->num,1);
    
    printf("here: %s:%d\n",__func__,__LINE__);

    pa->files[i] = NULL;
    pa->p[i].fd = fd; pa->p[i].events = mode; pa->p[i].revents = 0;
    pa->data[i] = data;
    pa->group[i] = pg;

    // resize arrays in the associated pollgroup
    int j = pg->num;

    printf("here: %s:%d\n",__func__,__LINE__);

    RESIZE_SIZE(size, pg->num, 1, DEFAULT_PA_GRAN) {
        printf("here: %s:%d gsize=%d\n",__func__,__LINE__,size);
        RESIZE(pg->rfds, size);
        RESIZE(pg->wfds, size);
        RESIZE(pg->rfiles, size);
        RESIZE(pg->wfiles, size);
        RESIZE(pg->rdata, size);
        RESIZE(pg->wdata, size);
    } RESIZE_SIZE_FOOT(pg->num,1);

    printf("here: %s:%d\n",__func__,__LINE__);


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
    printf("here: %s:%d\n",__func__,__LINE__);
    if (timeout<0) timeout = DEFAULT_POLL_TIMEOUT;

    int sel = poll(pa->p, pa->num, timeout);
    if (sel<0) LH_ERROR(-1, "poll on %d descriptors failed", pa->num);
    if (sel == 0) return 0;

    printf("here: %s:%d\n",__func__,__LINE__);
    int i;

    // clear all groups
    //FIXME: optimize?
    for (i=0; i<pa->num; i++) {
        pa->group[i]->rnum = 0;
        pa->group[i]->wnum = 0;
    }

    printf("here: %s:%d\n",__func__,__LINE__);
    // distribute events
    for (i=0; i<pa->num; i++) {
        pollgroup * pg = pa->group[i];
        printf("here: %s:%d i=%d pg=%08p\n",__func__,__LINE__,i,pg);
        printf("Dumping pg:\n"
               " num=%d\n"
               " rfds=%08p\n"
               " rfiles=%08p\n"
               " rdata=%08p\n"
               " rnum=%d\n",
               pg->num,pg->rfds,pg->rfiles,pg->rdata,pg->rnum);
        if (pa->p[i].revents & POLLIN) {
            printf("here: %s:%d\n",__func__,__LINE__);
            pg->rfds[pg->rnum]   = pa->p[i].fd;
            pg->rfiles[pg->rnum] = pa->files[i];
            pg->rdata[pg->rnum]  = pa->data[i];
            pg->rnum++;
            printf("here: %s:%d\n",__func__,__LINE__);
        }
        if (pa->p[i].revents & POLLOUT) {
            printf("here: %s:%d\n",__func__,__LINE__);
            pg->wfds[pg->wnum]   = pa->p[i].fd;
            pg->wfiles[pg->wnum] = pa->files[i];
            pg->wdata[pg->wnum]  = pa->data[i];
            pg->wnum++;
            printf("here: %s:%d\n",__func__,__LINE__);
        }
    }
    printf("here: %s:%d\n",__func__,__LINE__);
    return 0;
}

