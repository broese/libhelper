/*! \file
 * File functions
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
#define LH_READ_SIZELESS_FACTOR 4
#endif

#ifndef LH_BUF_DEFAULT_GRAN
#define LH_BUF_DEFAULT_GRAN 65536
#endif

typedef struct _lh_buf_t {
    uint8_t * data_ptr;
    ssize_t   data_cnt;  // this is actually the "writing index"
                         // maybe we can define the alternative macro AR() in the scope of lh_files
    ssize_t   data_ridx; // read index - this can be used by the write functions
    int       data_gran;
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

#define lh_read_static(...) lh_read_static_at(##__VA_ARGS__,-1,-1)
#define lh_read_alloc(...)  lh_read_alloc_at(##__VA_ARGS__,-1,-1)
#define lh_read_buf(...)    lh_read_buf_at(##__VA_ARGS__,-1,-1)

#define lh_load_static(path, ...) ( {                                   \
            int fd = lh_open_read(path, NULL);                          \
            ssize_t rlen = LH_FILE_ERROR;                               \
            if (fd >= 0) {                                              \
                rlen = lh_read_static_at(fd, ##__VA_ARGS__,-1,-1);      \
                close(fd);                                              \
            }                                                           \
            rlen; } )

#define lh_load_alloc(path, ...) ( {                                    \
            int fd = lh_open_read(path, NULL);                          \
            ssize_t rlen = LH_FILE_ERROR;                               \
            if (fd >= 0) {                                              \
                rlen = lh_read_alloc_at(fd, ##__VA_ARGS__,-1,-1);       \
                close(fd);                                              \
            }                                                           \
            rlen; } )

#define lh_load_buf(path, ...) ( {                                      \
            int fd = lh_open_read(path, NULL);                          \
            ssize_t rlen = LH_FILE_ERROR;                               \
            if (fd >= 0) {                                              \
                rlen = lh_read_buf_at(fd, ##__VA_ARGS__,-1,-1);         \
                close(fd);                                              \
            }                                                           \
            rlen; } )

#define lh_fpread_static(fp,...) lh_read_static(fileno(fp),__VA_ARGS__)
#define lh_fpread_alloc(fp,...)  lh_read_alloc(fileno(fp),__VA_ARGS__)
#define lh_fpread_buf(fp,...)    lh_read_buf(fileno(fp),__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////

ssize_t lh_write_at(int fd, uint8_t *buf, ssize_t length, off_t offset, ...);
ssize_t lh_write_buf_at(int fd, lh_buf_t *bo, ssize_t length, off_t offset, ...);

#define lh_write(...)            lh_write_at(##__VA_ARGS__,-1,-1)
#define lh_write_buf(...)        lh_write_buf_at(##__VA_ARGS__,-1,-1)

#define lh_save(path, ...) ( {                                          \
            int fd = lh_open_write(path);                               \
            ssize_t wlen = LH_FILE_ERROR;                               \
            if (fd >= 0) {                                              \
                wlen = lh_write_at(fd, ##__VA_ARGS__,-1,-1);            \
                close(fd);                                              \
            }                                                           \
            wlen; } )

#define lh_save_buf(path, ...) ( {                                      \
            int fd = lh_open_write(path);                               \
            ssize_t wlen = LH_FILE_ERROR;                               \
            if (fd >= 0) {                                              \
                wlen = lh_write_buf_at(fd, ##__VA_ARGS__,-1,-1);        \
                close(fd);                                              \
            }                                                           \
            wlen; } )

#define lh_fwrite(fp,...)        lh_write(fileno(fp),__VA_ARGS__)
#define lh_fwrite_buf(fp,...)    lh_write_buf(fileno(fp),__VA_ARGS__)










#if 0

////////////////////////////////////////////////////////////////////////////////
/// @name Writing files

/*! \brief Write a data block to a file, creating a new file in process
 * \param path File path
 * \param data Buffer with the data to write to the file
 * \param size Size to write, in bytes
 * \return 0 on success, -1 on error */
int lh_write_file(const char *path, const uint8_t *data, ssize_t size);

/*! \brief Write a data block to a file at specified position and keeping other data
 * \param path File path
 * \param pos Absolute position in the file to start writing at
 * \param data Buffer with the data to write to the file
 * \param size Size to write, in bytes
 * \return 0 on success, -1 on error */
int lh_write_file_at(const char *path, off_t pos, const uint8_t *data, ssize_t size);

/*! \brief Write a data block at current writing position in an opened file
 * \param fp File descriptor, must be opened for writing
 * \param data Buffer with the data to write to the file
 * \param size Size to write, in bytes
 * \return 0 on success, -1 on error */
int lh_write_fp(FILE *fp, const uint8_t *data, ssize_t size);

/*! \brief Write a data block at specified position in an opened file
 * \param fp File descriptor, must be opened for writing
 * \param pos Absolute position in the file to start writing at
 * \param data Buffer with the data to write to the file
 * \param size Size to write, in bytes
 * \return 0 on success, -1 on error */
int lh_write_fp_at(FILE *fp, off_t pos, const uint8_t *data, ssize_t size);

/*! \brief Append a data block to an existing file, or create new if does not exist
 * \param path File path
 * \param data Buffer with the data to write to the file
 * \param size Size to write, in bytes
 * \return 0 on success, -1 on error */
int lh_append_file(const char *path, const uint8_t *data, ssize_t size);

#ifdef LH_DECLARE_SHORT_NAMES

#define fopen_r(path,size)  lh_open_file_read(path,size)
#define fopen_w(path)       lh_open_file_write(path)
#define fopen_u(path,size)  lh_open_file_update(path,size)
#define fopen_a(path,size)  lh_open_file_append(path,size)

#define read_fp(fp,size,buffer)         lh_read_fp(fp,size,buffer)
#define read_fp_at(fp,pos,size,buffer)  lh_read_fp_at(fp,pos,size,buffer)
#define load_fp(fp,sizep)               lh_load_fp(fp,sizep)
#define read_file(path,size,buffer)     lh_read_file(path,size,buffer)
#define read_file_at(path,pos,size,buffer)  lh_read_file_at(path,pos,size,buffer)
#define load_file(path,sizep)           lh_load_file(path,sizep)

#define write_file(path,data,size)      lh_write_file(path,data,size)
#define write_file_at(path,pos,data,size)   lh_write_file(path,pos,data,size)
#define write_fp(fp,data,size)          lh_write_fp(fp,data,size)
#define write_fp_at(fp,pos,data,size)   lh_write_fp(fp,pos,data,size)
#define append_file(path,data,size)     lh_append_file(path,data,size)

#endif

#endif
