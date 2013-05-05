#pragma once

/*
lh_files - file operations
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////

// stat-related functions
off_t filesize(const char * path);
off_t filesize_fp(FILE * fd);

////////////////////////////////////////////////////////////////////////////////
// file opening

// open for reading
FILE *open_file_r(const char *path, off_t *size);

// open for writing (file created or truncated)
FILE *open_file_w(const char *path);

// open for updating
FILE *open_file_u(const char *path, off_t *size);

// open for appending - write position set at the end of file
FILE *open_file_a(const char *path, off_t *size);

////////////////////////////////////////////////////////////////////////////////
// reding files

// read a segment of file from opened descriptor at current position into allocated buffer
uint8_t *  read_fp(FILE *fp, ssize_t size);
//fp     : file descriptor
//size   : size to read
//return : allocated buffer for data, NULL if read failed

// read a segment of file from opened descriptor at position into allocated buffer
uint8_t *  read_fp_at(FILE *fp, off_t pos, ssize_t size);
//fp     : file descriptor
//pos    : file position to read from
//size   : size to read
//return : allocated buffer for data, NULL if read failed

// read the entire file from opened descriptor into allocated buffer
uint8_t *  read_fp_whole(FILE *fp, ssize_t *size);
//fp     : file descriptor
//size   : ssize_t variable to place file size into, NULL to ignore
//return : allocated buffer for data, NULL if read failed

// read the entire file from path into allocated buffer
uint8_t *  read_file_whole(const char * path, ssize_t *size);
//path   : file path to open
//size   : ssize_t variable to place file size into, NULL to ignore
//return : allocated buffer for data, NULL if read failed

// read a segment of file from path at specific position into allocated buffer
uint8_t *  read_file_at(const char * path, off_t pos, ssize_t size);
//path   : file path to open
//pos    : file position to read from
//size   : size to read
//return : allocated buffer for data, NULL if read failed

// read a segment of file from opened descriptor at current position into user buffer
int        read_fp_to(FILE *fp, uint8_t * buffer, ssize_t size);
//fp     : file descriptor
//buffer : buffer to write data to
//size   : size to read
//return : 0 on success, -1 on error

// read a segment of file from opened descriptor at specific position into user buffer
int        read_fp_at_to(FILE *fp, uint8_t * buffer, off_t pos, ssize_t size);
//fp     : file descriptor
//buffer : buffer to write data to
//pos    : file position to read from
//size   : size to read
//return : 0 on success, -1 on error

// read a segment of file from path at specific position into user buffer
int        read_file_at_to(const char * path, uint8_t * buffer, off_t pos, ssize_t size);
//path   : file path to open
//buffer : buffer to write data to
//pos    : file position to read from
//size   : size to read
//return : 0 on success, -1 on error

// write a data block to a file, creating a new file in process
int        write_file(const char *path, const uint8_t *data, ssize_t size);
//path   : file path to open
//data   : buffer with the data to write to the file
//size   : size of data to write

// write a data block to a file at specified position and keeping other data
int        write_file_at(const char *path, off_t pos, const uint8_t *data, ssize_t size);
//path   : file path to open
//pos    : absolute position in the file to start writing at
//data   : buffer with the data to write to the file
//size   : size of data to write

// write a data block at current writing position in an opened file
int        write_fp(FILE *fp, const uint8_t *data, ssize_t size);
//fp     : file descriptor, opened for writing
//data   : buffer with the data to write to the file
//size   : size of data to write

// write a data block at specified position in an opened file
int        write_fp_at(FILE *fp, off_t pos, const uint8_t *data, ssize_t size);
//fp     : file descriptor, opened for writing
//pos    : absolute position in the file to start writing at
//data   : buffer with the data to write to the file
//size   : size of data to write

// append a data block to an existing file, or create new if does not exist
int        append_file(const char *path, const uint8_t *data, ssize_t size);
//path   : file path to open
//data   : buffer with the data to write to the file
//size   : size of data to write

#if 0
int write_file(const char *path, const unsigned char *data, ssize_t size);
int write_file_f(FILE *fd, const unsigned char *data, ssize_t size);
int append_file(const char *path, const unsigned char *data, ssize_t size);
#endif
