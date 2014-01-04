/*! \file
 * File functions
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////

// stat-related functions
/*! \brief Return the size of a file specified by path
 * \param path Path of the file (relative or absolute)
 * \return Size of the file in bytes, -1 on error. */
off_t filesize(const char * path);

/*! \brief Return the size of a file specified by an open file descriptor
 * \param path File descriptor (
 * \return Size of the file in bytes, -1 on error. */
off_t filesize_fp(FILE * fd);

////////////////////////////////////////////////////////////////////////////////
// file opening

/*! \brief Open file at specific path for reading
 * File is readable only, read position is set at the beginning
 * \param path Path of the file (relative or absolute)
 * \param size Current size will be stored in the variable referenced by this pointer
 * \return File descriptor, NULL on error. */
FILE *open_file_r(const char *path, off_t *size);

/*! \brief Open file at specific path for writing.
 * File is writable only, file is created or truncated, write position is set at the beginning
 * \param path Path of the file (relative or absolute)
 * \return File descriptor, NULL on error. */
FILE *open_file_w(const char *path);

/*! \brief Open file at specific path for updating. 
 * File is both readable and writable, read/write position is set at the beginning
 * \param path Path of the file (relative or absolute)
 * \param size Current size will be stored in the variable referenced by this pointer
 * \return File descriptor, NULL on error. */
FILE *open_file_u(const char *path, off_t *size);

/*! \brief Open file at specific path for appending
 * File is writable only, write position is set at the end
 * \param path Path of the file (relative or absolute)
 * \param size Current size will be stored in the variable referenced by this pointer
 * \return File descriptor, NULL on error. */
FILE *open_file_a(const char *path, off_t *size);

////////////////////////////////////////////////////////////////////////////////
// reding files

/*! \brief Read a segment of file from opened descriptor at current position into allocated buffer
 * \param fp File descriptor
 * \param size Size to read, in bytes
 * \return allocated buffer for data, NULL if read failed */
uint8_t *  read_fp(FILE *fp, ssize_t size);

/*! \brief Read a segment of file from opened descriptor at position into allocated buffer
 * \param fp File descriptor
 * \param pos File position to read from
 * \param size Size to read, in bytes
 * \return allocated buffer for data, NULL if read failed */
uint8_t *  read_fp_at(FILE *fp, off_t pos, ssize_t size);

/*! \brief Read the entire file from opened descriptor into allocated buffer
 * \param fp File descriptor
 * \param size Pointer to a ssize_t variable to place file size into, NULL to ignore
 * \return allocated buffer for data, NULL if read failed */
uint8_t *  read_fp_whole(FILE *fp, ssize_t *size);

/*! \brief Read the entire file from path into allocated buffer
 * \param path File path
 * \param size Pointer to a ssize_t variable to place file size into, NULL to ignore
 * \return allocated buffer for data, NULL if read failed */
uint8_t *  read_file_whole(const char * path, ssize_t *size);

/*! \brief Read a segment of file from path at specific position into allocated buffer
 * \param path File path
 * \param pos File position to read from
 * \param size Size to read, in bytes
 * \return allocated buffer for data, NULL if read failed */
uint8_t *  read_file_at(const char * path, off_t pos, ssize_t size);

/*! \brief read a segment of file from opened descriptor at current position into user buffer
 * \param fp File descriptor
 * \param buffer Buffer to write data to
 * \param size Size to read, in bytes
 * \return 0 on success, -1 on error */
int        read_fp_to(FILE *fp, uint8_t * buffer, ssize_t size);

/*! \brief Read a segment of file from opened descriptor at specific position into user buffer
 * \param fp File descriptor
 * \param buffer Buffer to write data to
 * \param pos File position to read from
 * \param size Size to read, in bytes
 * \return 0 on success, -1 on error */
int        read_fp_at_to(FILE *fp, uint8_t * buffer, off_t pos, ssize_t size);

/*! \brief Read a segment of file from path at specific position into user buffer
 * \param path File path
 * \param buffer Buffer to write data to
 * \param pos File position to read from
 * \param size Size to read, in bytes
 * \return 0 on success, -1 on error */
int        read_file_at_to(const char * path, uint8_t * buffer, off_t pos, ssize_t size);

/*! \brief Write a data block to a file, creating a new file in process
 * \param path File path
 * \param data Buffer with the data to write to the file
 * \param size Size to write, in bytes
 * \return 0 on success, -1 on error */
int        write_file(const char *path, const uint8_t *data, ssize_t size);

/*! \brief Write a data block to a file at specified position and keeping other data
 * \param path File path
 * \param pos Absolute position in the file to start writing at
 * \param data Buffer with the data to write to the file
 * \param size Size to write, in bytes
 * \return 0 on success, -1 on error */
int        write_file_at(const char *path, off_t pos, const uint8_t *data, ssize_t size);

/*! \brief Write a data block at current writing position in an opened file
 * \param fp File descriptor, must be opened for writing
 * \param data Buffer with the data to write to the file
 * \param size Size to write, in bytes
 * \return 0 on success, -1 on error */
int        write_fp(FILE *fp, const uint8_t *data, ssize_t size);

/*! \brief Write a data block at specified position in an opened file
 * \param fp File descriptor, must be opened for writing
 * \param pos Absolute position in the file to start writing at
 * \param data Buffer with the data to write to the file
 * \param size Size to write, in bytes
 * \return 0 on success, -1 on error */
int        write_fp_at(FILE *fp, off_t pos, const uint8_t *data, ssize_t size);

/*! \brief Append a data block to an existing file, or create new if does not exist
 * \param path File path
 * \param data Buffer with the data to write to the file
 * \param size Size to write, in bytes
 * \return 0 on success, -1 on error */
int        append_file(const char *path, const uint8_t *data, ssize_t size);

#if 0
int write_file(const char *path, const unsigned char *data, ssize_t size);
int write_file_f(FILE *fd, const unsigned char *data, ssize_t size);
int append_file(const char *path, const unsigned char *data, ssize_t size);
#endif
