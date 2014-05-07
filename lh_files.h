/*! \file
 * File functions
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "lh_arr.h"

#ifndef LH_FILE_CREAT_MODE
#define LH_FILE_CREAT_MODE 0777
#endif


////////////////////////////////////////////////////////////////////////////////
/// @name Stat-related functions

off_t lh_filesize(int fd);
off_t lh_filesize_path(const char * path);
#define lh_filesize_fp(fp) lh_filesize(fileno(fp))

////////////////////////////////////////////////////////////////////////////////
/// @name Opening files

int lh_open_read(const char *path, off_t *sizep);
int lh_open_write(const char *path);
int lh_open_update(const char *path, off_t *sizep);
int lh_open_append(const char *path, off_t *sizep);

////////////////////////////////////////////////////////////////////////////////
/// @name Reading files

#define LH_FILE_WAIT     0  // read/write would block - try again later
#define LH_FILE_EOF     -1  // end of file reached
#define LH_FILE_INVALID -2  // invalid parameters supplied
#define LH_FILE_ERROR   -3  // error writing/reading file

#ifndef LH_READ_SIZELESS_FACTOR
#define LH_READ_SIZELESS_FACTOR 16
#endif

#ifndef LH_BUF_DEFAULT_GRAN
#define LH_BUF_DEFAULT_GRAN 65536
#endif

typedef struct _lh_buf_t {
    lh_buf_declare(data);
    ssize_t ridx; // read index - this can be used by the write functions
} lh_buf_t;

ssize_t lh_read_static_at(int fd, uint8_t *buf, ssize_t length, off_t offset, ...);
ssize_t lh_read_alloc_at(int fd, uint8_t **bufp, ssize_t length, off_t offset, ...);
ssize_t lh_read_buf_at(int fd, lh_buf_t *bo, ssize_t length, off_t offset, ...);

/**
 * = file source types =
 * lh_read_*  : read from a file descriptor (int)
 * lh_fread_* : read from a file pointer (FILE *)
 * lh_load_*  : read from a path, opening and closing the file in process
 *
 * = destination buffer types =
 * *_static : read into a static buffer (in this case length is mandatory!)
 * *_alloc  : alloc a buffer of appropriate size and return it to the user
 *            via uint8_t ** bufp variable
 * *_buf    : use a lh_buf_t struct to manage the buffer. The user supplies
 *            a pointer to lh_buf_t - it can be zeroed to initialize an
 *            empty buffer. Unlike other functions, this read function will
 *            append data to the existing data in the buffer, automatically
 *            resizing it as needed
 *
 * length   : specifies the amount of data to read. This parameter is
 *            optional, you can omit it when calling one of the wrapper
 *            macros (you will need to omit the following offset as well)
 *            or set it to -1 to signify omission. If the length is not
 *            specified, the functions will attempt to read the entire file.
 *            In case of the size-less files, such as STDIN, pipes, sockets,
 *            etc., the function will attempt to read at most
 *            LH_READ_SIZELESS_FACTOR * granularity bytes, aligned to the
 *            next granularity boundary.
 *
 * offset   : specifies the offset to read from. This value is optional,
 *            if omitted in the wrapping macros, or given as -1, the method
 *            will read from the current position, for lh_load_ functions
 *            this implies - from the beginning. If specified, but the file
 *            is not seekable, it will generate an error.
 */

#define lh_read_static(...) lh_read_static_at(__VA_ARGS__,-1,-1)
#define lh_read_alloc(...)  lh_read_alloc_at(__VA_ARGS__,-1,-1)
#define lh_read_buf(...)    lh_read_buf_at(__VA_ARGS__,-1,-1)

#define lh_load_static(path, ...) ( {                                   \
            int fd = lh_open_read(path, NULL);                          \
            ssize_t rlen = LH_FILE_ERROR;                               \
            if (fd >= 0) {                                              \
                rlen = lh_read_static_at(fd,__VA_ARGS__,-1,-1);         \
                close(fd);                                              \
            }                                                           \
            rlen; } )

#define lh_load_alloc(path, ...) ( {                                    \
            int fd = lh_open_read(path, NULL);                          \
            ssize_t rlen = LH_FILE_ERROR;                               \
            if (fd >= 0) {                                              \
                rlen = lh_read_alloc_at(fd,__VA_ARGS__,-1,-1);          \
                close(fd);                                              \
            }                                                           \
            rlen; } )

#define lh_load_buf(path, ...) ( {                                      \
            int fd = lh_open_read(path, NULL);                          \
            ssize_t rlen = LH_FILE_ERROR;                               \
            if (fd >= 0) {                                              \
                rlen = lh_read_buf_at(fd,__VA_ARGS__,-1,-1);            \
                close(fd);                                              \
            }                                                           \
            rlen; } )

#define lh_fread_static(fp,...) lh_read_static(fileno(fp),__VA_ARGS__)
#define lh_fread_alloc(fp,...)  lh_read_alloc(fileno(fp),__VA_ARGS__)
#define lh_fread_buf(fp,...)    lh_read_buf(fileno(fp),__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////

ssize_t lh_write_at(int fd, uint8_t *buf, ssize_t length, off_t offset, ...);
ssize_t lh_write_buf_at(int fd, lh_buf_t *bo, ssize_t length, off_t offset, ...);

#define lh_write(...)           lh_write_at(__VA_ARGS__,-1,-1)
#define lh_write_buf(...)       lh_write_buf_at(__VA_ARGS__,-1,-1)

#define lh_save(path, ...) ( {                                          \
            int fd = lh_open_write(path);                               \
            ssize_t wlen = LH_FILE_ERROR;                               \
            if (fd >= 0) {                                              \
                wlen = lh_write_at(fd,__VA_ARGS__,-1,-1);               \
                close(fd);                                              \
            }                                                           \
            wlen; } )

#define lh_save_buf(path, ...) ( {                                      \
            int fd = lh_open_write(path);                               \
            ssize_t wlen = LH_FILE_ERROR;                               \
            if (fd >= 0) {                                              \
                wlen = lh_write_buf_at(fd,__VA_ARGS__,-1,-1);           \
                close(fd);                                              \
            }                                                           \
            wlen; } )

#define lh_fwrite(fp,...)       lh_write(fileno(fp),__VA_ARGS__)
#define lh_fwrite_buf(fp,...)   lh_write_buf(fileno(fp),__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////

ssize_t lh_append(int fd, uint8_t *buf, ssize_t length);
ssize_t lh_pappend(const char *path, uint8_t *buf, ssize_t length);
#define lh_fappend(fp, buf, length) lh_append(fileno(fp), buf, length)

ssize_t lh_append_buf_l(int fd, lh_buf_t *bo, ssize_t length, ...);
ssize_t lh_pappend_buf_l(const char *path, lh_buf_t *bo, ssize_t length, ...);

#define lh_append_buf(fd, bo, ...) lh_append_buf_l(fd, bo, ##__VA_ARGS__, -1)
#define lh_pappend_buf(path, bo, ...) lh_pappend_buf_l(path, bo, ##__VA_ARGS__, -1)
#define lh_fappend_buf(fp, bo, ...) lh_append_buf_l(fileno(fp), bo, ##__VA_ARGS)

////////////////////////////////////////////////////////////////////////////////

#ifdef LH_DECLARE_SHORT_NAMES

#define filesize        lh_filesize
#define filesize_p      lh_filesize_path
#define filesize_fp     lh_filesize_fp

#define open_r          lh_open_read
#define open_w          lh_open_write
#define open_u          lh_open_update
#define open_a          lh_open_append

#define read_s          lh_read_static
#define read_a          lh_read_alloc
#define read_b          lh_read_buf
#define fread_s         lh_fread_static
#define fread_a         lh_fread_alloc
#define fread_b         lh_fread_buf
#define load_s          lh_load_static
#define load_a          lh_load_alloc
#define load_b          lh_load_buf

#define write_s         lh_write
#define write_b         lh_write_buf
#define fwrite_s        lh_fwrite
#define fwrite_b        lh_fwrite_buf
#define save_s          lh_save
#define save_b          lh_save_buf

#define append_s        lh_append
#define append_b        lh_append_buf
#define pappend_s       lh_path_append
#define pappend_b       lh_path_append_buf
#define fappend_s       lh_fappend
#define fappend_b       lh_fappend_buf

#endif
