#pragma once 

#include <stdlib.h>

/*
lh_compress - support for compression/decompression of data
*/

////////////////////////////////////////////////////////////////////////////////

unsigned char * zlib_encode(const unsigned char *data, ssize_t length, ssize_t *olength);
unsigned char * zlib_decode(const unsigned char *data, ssize_t length, ssize_t *olength);

unsigned char * gzip_encode(const unsigned char *data, ssize_t length, ssize_t *olength);
unsigned char * gzip_decode(const unsigned char *data, ssize_t length, ssize_t *olength);

