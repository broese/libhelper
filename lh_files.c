/*
 Authors:
 Copyright 2012-2015 by Eduard Broese <ed.broese@gmx.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version
 2 of the License, or (at your option) any later version.
*/

#include "lh_files.h"
#include "lh_buffers.h"
#include "lh_debug.h"

#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#ifdef HAVE_BLKGETSIZE64
#include <sys/ioctl.h>
#include <linux/fs.h>
#endif

////////////////////////////////////////////////////////////////////////////////

//TODO: handle non-regular files

off_t lh_filesize(int fd) {
    struct stat st;
    if (fd<0) LH_ERROR(-1,"Incorrect file descriptor");
    if (fstat(fd, &st)) LH_ERROR(-1,"Failed to stat file descriptor %d",fd);

    if (S_ISREG(st.st_mode)) return st.st_size;

#ifdef HAVE_BLKGETSIZE64
    if (S_ISBLK(st.st_mode)) {
        off_t nbytes;
        if (ioctl(fd, BLKGETSIZE64, &nbytes)<0)
            LH_ERROR(-1,"ioctl(BLKGETSIZE64) failed to FD %d",fd);
        return nbytes;
    }
#endif

    //LH_ERROR(-1,"Cannot obtain size for this file type %d",fd);
    return -1;
}

off_t lh_filesize_path(const char * path) {
    struct stat st;
    if (stat(path, &st)) LH_ERROR(-1,"Failed to stat file %s",path);

    if (S_ISREG(st.st_mode)) return st.st_size;

#ifdef HAVE_BLKGETSIZE64
    if (S_ISBLK(st.st_mode)) {
        int fd = open(path, O_RDONLY);
        if (fd<0) LH_ERROR(-1,"Failed to get read access to %s",path);

        off_t nbytes;
        if (ioctl(fd, BLKGETSIZE64, &nbytes)<0)
            LH_ERROR(-1,"ioctl(BLKGETSIZE64) failed to %s",path);
        close(fd);
        return nbytes;
    }
#endif

    //LH_ERROR(-1,"Cannot obtain size for this file type %s",path);
    return -1;
}

////////////////////////////////////////////////////////////////////////////////

int lh_path_exists(const char *path) {
    struct stat st;
    if (stat(path, &st)) return 0;
    return 1;
}

int lh_path_isfile(const char *path) {
    struct stat st;
    if (stat(path, &st)) return 0;
    if ( (st.st_mode & S_IFMT) == S_IFREG ) return 1;
    return 0;
}

int lh_path_isdir(const char *path) {
    struct stat st;
    if (stat(path, &st)) return 0;
    if ( (st.st_mode & S_IFMT) == S_IFDIR ) return 1;
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

static int lh_open_file(const char *path, off_t *sizep, int flags, const char *smode) {
#ifdef O_LARGEFILE
    flags |= O_LARGEFILE;
#endif

    int fd;
    if (flags & O_CREAT)
        fd = open(path, flags, LH_FILE_CREAT_MODE);
    else
        fd = open(path, flags);

    if (sizep) *sizep = -1;
    if (fd<0) LH_ERROR(-1,"Failed to open %s for %s",path,smode);
    if (sizep) *sizep = lh_filesize(fd);

    return fd;
}

int lh_open_read(const char *path, off_t *sizep) {
    return lh_open_file(path,sizep,O_RDONLY,"reading");
}

int lh_open_write(const char *path) {
    return lh_open_file(path,NULL,O_WRONLY|O_CREAT|O_TRUNC,"writing");
}

int lh_open_update(const char *path, off_t *sizep) {
    return lh_open_file(path,sizep,O_RDWR|O_CREAT,"updating");
}

int lh_open_append(const char *path, off_t *sizep) {
    return lh_open_file(path,sizep,O_WRONLY|O_CREAT|O_APPEND,"appending");
}

////////////////////////////////////////////////////////////////////////////////

ssize_t lh_read_static_at(int fd, uint8_t *buf, ssize_t length, off_t offset, ...) {
    // when static buffer is used, the user MUST supply length
    if (!buf || fd<0 || length<0) return LH_FILE_INVALID;

    // seek to position if the offset parameter is supplied
    if (offset >= 0) {
        // note that the file might be unseekable, such as a pipe or socket,
        // which will throw an error
        if (lseek(fd, offset, SEEK_SET)<0)
            LH_ERROR(LH_FILE_ERROR, "Failed to seek to offset %jd", (intmax_t) offset);
    }

    // attempt to read
    ssize_t rbytes = read(fd, buf, length);

    if (rbytes == 0)
        return LH_FILE_EOF;

    if (rbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return LH_FILE_WAIT;
        else
            return LH_FILE_ERROR;
    }

    return rbytes;
}

ssize_t lh_read_buf_at(int fd, lh_buf_t *bo, ssize_t length, off_t offset, ...) {
    if (!bo || fd<0) return LH_FILE_INVALID;

    // if no length was supplied, attempt to determine how much will be read
    ssize_t rsize = length;
    if (length < 0) {
        off_t fsize = lh_filesize(fd);
        //FIXME: -1 may mean the file is sizeless (still OK to read)
        //       or there was an error (not OK). Find a better way to
        //       differentiate these two conditions. For now, we just
        //       continue, and if the read fails, it fails.
        if (fsize < 0) {
            // the file size cannot be determined, we will assume the default
            // LH_READ_SIZELESS_FACTOR * granularity,
            rsize = LH_READ_SIZELESS_FACTOR * LH_BUF_DEFAULT_GRAN;

            // trim it to the next granularity boundary, so we get a clean total size
            ssize_t rem = C(bo->data) & (LH_BUF_DEFAULT_GRAN-1);
            rsize -= rem;
        }
        else {
            // the file size is known, determine how much we will need to read
            // from the current or intended offset
            off_t curoffset = offset;
            if (offset < 0)
                curoffset = lseek(fd, 0, SEEK_CUR);
            //FIXME: sanity checks, what if the offset is past the file end?
            rsize = fsize - curoffset;
        }
    }

    // resize the buffer
    ssize_t widx = C(bo->data);
    lh_arr_add(GAR4(bo->data), rsize);

    // read as in a static buffer, then adjust the buffer info
    ssize_t rbytes = lh_read_static_at(fd, P(bo->data)+widx, rsize, offset);
    if (rbytes < 0)
        C(bo->data) = widx;
    else
        C(bo->data) = widx + rbytes;

    return rbytes;
}

ssize_t lh_read_alloc_at(int fd, uint8_t **bufp, ssize_t length, off_t offset, ...) {
    if (!bufp) return LH_FILE_INVALID;

    lh_buf_t bo;
    lh_clear_obj(bo);

    ssize_t rbytes = lh_read_buf_at(fd, &bo, length, offset);
    if (rbytes < 0) {
        if (P(bo.data)) free(P(bo.data));
        *bufp = NULL;
    }
    else {
        *bufp = P(bo.data);
    }
    return rbytes;
}

////////////////////////////////////////////////////////////////////////////////

ssize_t lh_write_at(int fd, uint8_t *buf, ssize_t length, off_t offset, ...) {
    // when static buffer is used, the user MUST supply length
    if (!buf || fd<0 || length<0) return LH_FILE_INVALID;

    // seek to position if the offset parameter is supplied
    if (offset >= 0) {
        // note that the file might be unseekable, such as a pipe or socket,
        // which will throw an error
        if (lseek(fd, offset, SEEK_SET)<0)
            LH_ERROR(LH_FILE_ERROR, "Failed to seek to offset %jd", (intmax_t) offset);
    }

    // attempt to write
    ssize_t wbytes = write(fd, buf, length);

    if (wbytes < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return LH_FILE_WAIT;
        else
            return LH_FILE_ERROR;
    }

    return wbytes;
}

ssize_t lh_write_buf_at(int fd, lh_buf_t *bo, ssize_t length, off_t offset, ...) {
    if (!bo || fd<0) return LH_FILE_INVALID;

    ssize_t wsize = (length<0) ? (C(bo->data)-bo->ridx) : length;
    ssize_t wbytes = lh_write_at(fd, P(bo->data)+bo->ridx, wsize, offset);

    if (wbytes >= 0) {
        bo->ridx+=wbytes;
        wsize = C(bo->data)-bo->ridx;
        if (wsize > 0) {
            // some data remains in the buffer, should we compact it?
            if (lh_align(C(bo->data),LH_BUF_DEFAULT_GRAN) >
                lh_align(bo->ridx,LH_BUF_DEFAULT_GRAN) &&
                wsize < LH_BUF_DEFAULT_GRAN ) {
                lh_move(P(bo->data), bo->ridx, 0, wsize);
                bo->ridx=0;
                C(bo->data)=wsize;
            }
        }
        else {
            // all data was written, reset the positions in the buffer object
            C(bo->data)=0;
            bo->ridx=0;
        }
    }

    return wbytes;
}

////////////////////////////////////////////////////////////////////////////////

ssize_t lh_append(int fd, uint8_t *buf, ssize_t length) {
    if (lseek(fd, 0, SEEK_END)<0) return LH_FILE_ERROR;
    return lh_write_at(fd, buf, length, -1);
}

ssize_t lh_pappend(const char *path, uint8_t *buf, ssize_t length) {
    int fd = lh_open_append(path, NULL);
    ssize_t result = lh_write_at(fd, buf, length, -1);
    close(fd);
    return result;
}

ssize_t lh_append_buf_l(int fd, lh_buf_t *bo, ssize_t length, ...) {
    if (lseek(fd, 0, SEEK_END)<0) return LH_FILE_ERROR;
    return lh_write_buf_at(fd, bo, length, -1);
}

ssize_t lh_pappend_buf_l(const char *path, lh_buf_t *bo, ssize_t length, ...) {
    int fd = lh_open_append(path, NULL);
    ssize_t result = lh_write_buf_at(fd, bo, length, -1);
    close(fd);
    return result;
}

