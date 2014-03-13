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

typedef struct _lh_buf_t {
    uint8_t * data_ptr;
    ssize_t   data_cnt;  // this is actually the "writing index"
                         // maybe we can define the alternative macro AR() in the scope of lh_files
    ssize_t   data_ridx; // read index - this can be used by the write functions
    int       data_gran;
} lh_buf_t;

ssize_t lh_read_static_at(int fd, uint8_t *buf, ssize_t length, off_t offset);
ssize_t lh_read_alloc_at(int fd, uint8_t **bufp, ssize_t length, off_t offset);
ssize_t lh_read_buf_at(int fd, lh_buf_t *bo, ssize_t length, off_t offset);

#define lh_read_static(...) lh_read_static_at(##__VA_ARGS__,-1,-1)
#define lh_read_alloc(...)  lh_read_alloc_at(##__VA_ARGS__,-1,-1)
#define lh_read_buf(...)    lh_read_buf_at(##__VA_ARGS__,-1,-1)

#define lh_pread_static(path, ...) ( {                                  \
            int fd = lh_open_read(path, NULL);                          \
            ssize_t rlen = lh_read_static_at(fd, ##__VA_ARGS__,-1,-1);  \
            close(fd);                                                  \
            rlen; } )

#define lh_pread_alloc(path, ...) ( {                                   \
            int fd = lh_open_read(path, NULL);                          \
            ssize_t rlen = lh_read_alloc_at(fd, ##__VA_ARGS__,-1,-1);   \
            close(fd);                                                  \
            rlen; } )

#define lh_pread_buf(path, ...) ( {                                 \
            int fd = lh_open_read(path, NULL);                      \
            ssize_t rlen = lh_read_buf_at(fd, ##__VA_ARGS__,-1,-1); \
            close(fd);                                              \
            rlen; } )

#define lh_fpread_static(fp,...) lh_read_static(fileno(fp),__VA_ARGS__)
#define lh_fpread_alloc(fp,...)  lh_read_alloc(fileno(fp),__VA_ARGS__)
#define lh_fpread_buf(fp,...)    lh_read_buf(fileno(fp),__VA_ARGS__)












#if 0


// First decision - no allocation, only read to static buffer
// We decide later what to do with the allocation - possible approaches:
// 1. separate function with no buffer parameter
// 2. separate function with buffer pointer parameter
// 3. only work with resizable buffers

// Second decision - read only for opened file descriptors (fd or fp)
// Load only for paths - there seems to be little use for reading from the middle of 
// a file not opened yet
// Or maybe: have a function with offset and size, and a wrapper macro without
// Also - does it make sence to load into static buffer? I don't think so

// Third decision - load is only for allocation

/*! \brief Read a block from a file descriptor, to a static buffer.
 * \param fd File descriptor
 * \param buffer Buffer to store the data
 * \param size Number of bytes to read
 *             Buffer must offer at least that many bytes of storage
 * \param pos File offset to read from. <0 means - from current position
 * \return If >0 : Number of bytes actually read
 *         If <=0 : One of the LH_FILE_* constants
 */
ssize_t lh_read_at(int fd, uint8_t *buffer, ssize_t size, off_t pos);

#define lh_read(fd,buffer,size,...) lh_read_at(fd,buffer,size,##__VA_ARGS_,-1)

#define lh_read_fp(fp,buffer,size,...) lh_read_at(fileno(fp),buffer,size,##__VA_ARGS_,-1)

/*! \brief Read the entire file from a path, allocate the buffer for the data
 * \param path File path
 * \param bufp Pointer to the uint8_t variable where the buffer will be stored
 * \return The length of the read data
 */
ssize_t lh_load(const char *path, uint8_t **bufp);

/*
TODO:
lh_read_append - read a chunk of data, append to existing data in the buffer
lh_load_segment - read a file from a path, but only a chunk at given position and length

lh_read_buf - higher-level functions that use lh_buffer_t
lh_load_buf
*/

#endif









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
