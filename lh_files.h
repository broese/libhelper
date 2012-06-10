#pragma once

/*
lh_files - file operations
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////

off_t get_file_length(const char * path);
off_t get_file_length_f(FILE * fd);

FILE *open_file_r(const char *path, ssize_t *size);
FILE *open_file_w(const char *path);
FILE *open_file_u(const char *path, ssize_t *size);
FILE *open_file_a(const char *path, ssize_t *size);

unsigned char * read_file(const char * path, ssize_t *size);
unsigned char * read_file_f(FILE *fd, ssize_t *size);
int write_file(const char *path, const unsigned char *data, ssize_t size);
int write_file_f(FILE *fd, const unsigned char *data, ssize_t size);
int append_file(const char *path, const unsigned char *data, ssize_t size);

int write_to(FILE *fd, off_t offset, const unsigned char *data, ssize_t size);
int read_from(FILE *fd, off_t offset, unsigned char *buffer, ssize_t size);
unsigned char * read_froma(FILE *fd, off_t offset, ssize_t size);



