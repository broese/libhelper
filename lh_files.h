/*! \file
 * File functions
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////

/// @name Stat-related functions

/*! \brief Return the size of a file specified by path
 * \param path Path of the file (relative or absolute)
 * \return Size of the file in bytes, -1 on error. */
off_t lh_filesize_path(const char * path);

/*! \brief Return the size of a file specified by an open file descriptor
 * \param fp File descriptor
 * \return Size of the file in bytes, -1 on error. */
off_t lh_filesize_fp(FILE * fp);

////////////////////////////////////////////////////////////////////////////////
/// @name File Opening

/*! \brief Open file at specific path for reading
 * File is readable only, read position is set at the beginning
 * \param path Path of the file (relative or absolute)
 * \param sizep Pointer to an off_t variable where the size of the file will be
 * stored. NULL if not required.
 * \return File descriptor, NULL on error. */
FILE *lh_open_file_read(const char *path, off_t *sizep);

/*! \brief Open file at specific path for writing.
 * File is writable only, file is created or truncated, write position is set at the beginning
 * \param path Path of the file (relative or absolute)
 * \return File descriptor, NULL on error. */
FILE *lh_open_file_write(const char *path);

/*! \brief Open file at specific path for updating. 
 * File is both readable and writable, read/write position is set at the beginning
 * \param path Path of the file (relative or absolute)
 * \param sizep Pointer to an off_t variable where the size of the file will be
 * stored. NULL if not required.
 * \return File descriptor, NULL on error. */
FILE *lh_open_file_update(const char *path, off_t *sizep);

/*! \brief Open file at specific path for appending
 * File is writable only, write position is set at the end
 * \param path Path of the file (relative or absolute)
 * \param sizep Pointer to an off_t variable where the size of the file will be
 * stored. NULL if not required.
 * \return File descriptor, NULL on error. */
FILE *lh_open_file_append(const char *path, off_t *sizep);

////////////////////////////////////////////////////////////////////////////////
/// @name Reading files

/*! \brief Read a segment of file from opened descriptor at current position
 * \param fp File descriptor
 * \param size Size of data to read, in bytes
 * \param buffer Pointer to a buffer to store data.
 * If NULL, a buffer of appropriate size will be allocated
 * \return Pointer to the buffer with read data. NULL if an error occured.
 * If buffer was supplied (non-NULL), returned value will be identical
 * If buffer was not supplied, returned value is the allocated buffer, must be freed by caller
 */
uint8_t * lh_read_fp(FILE *fp, ssize_t size, uint8_t * buffer);

/*! \brief Read a segment of file from opened descriptor at specified position
 * \param fp File descriptor
 * \param pos File position to start reading from, in bytes
 * \param size Size of data to read, in bytes
 * \param buffer Pointer to a buffer to store data.
 * If NULL, a buffer of appropriate size will be allocated
 * \return Pointer to the buffer with read data. NULL if an error occured.
 * If buffer was supplied (non-NULL), returned value will be identical
 * If buffer was not supplied, returned value is the allocated buffer, must be freed by caller
 */
uint8_t * lh_read_fp_at(FILE *fp, off_t pos, ssize_t size, uint8_t * buffer);

/*! \brief Read the entire contents of a file from opened descriptor
 * \param fp File descriptor
 * \param sizep Pointer to a ssize_t variable, where the file length will be stored
 * \return Pointer to the buffer with read data. NULL if an error occured.
 * The returned buffer is allocated by the function, must be freed by the caller
 */
uint8_t * lh_load_fp(FILE *fp, ssize_t *sizep);

/*! \brief Read a segment of file specified by path, starting from offset 0
 * \param path File path
 * \param size Size of data to read, in bytes
 * \param buffer Pointer to a buffer to store data.
 * If NULL, a buffer of appropriate size will be allocated
 * \return Pointer to the buffer with read data. NULL if an error occured.
 * If <i>buffer</i> was supplied (non-NULL), returned value will be identical
 * If <i>buffer</i> was not supplied, returned value is the allocated buffer,
 * must be freed by the caller
 */
uint8_t * lh_read_file(const char *path, ssize_t size, uint8_t * buffer);

/*! \brief Read a segment of file specified by path, starting from specified offset
 * \param path File path
 * \param pos File position to start reading from, in bytes
 * \param size Size of data to read, in bytes
 * \param buffer Pointer to a buffer to store data.
 * If NULL, a buffer of appropriate size will be allocated
 * \return Pointer to the buffer with read data. NULL if an error occured.
 * If <i>buffer</i> was supplied (non-NULL), returned value will be identical
 * If <i>buffer</i> was not supplied, returned value is the allocated buffer,
 * must be freed by the caller
 */
uint8_t * lh_read_file_at(const char *path, off_t pos, ssize_t size, uint8_t * buffer);

/*! \brief Read the entire contents of a file specified by path
 * \param path File path
 * \param sizep Pointer to a ssize_t variable, where the file length will be stored
 * \return Pointer to the buffer with read data. NULL if an error occured.
 * The returned buffer is allocated by the function, must be freed by the caller
 */
uint8_t * lh_load_file(const char *path, ssize_t *sizep);

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
