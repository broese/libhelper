#pragma once 

#include <stdlib.h>
#include <stdint.h>

/*
lh_compress - support for compression/decompression of data
*/

#define LH_COMPRESS_ALLOCGRAN 4096

////////////////////////////////////////////////////////////////////////////////

uint8_t * zlib_encode(const uint8_t *ibuf, ssize_t ilen, ssize_t *olength);
uint8_t * zlib_decode(const uint8_t *ibuf, ssize_t ilen, ssize_t *olength);

ssize_t zlib_encode_to(const uint8_t *ibuf, ssize_t ilen, uint8_t *obuf, ssize_t olen);
ssize_t zlib_decode_to(const uint8_t *ibuf, ssize_t ilen, uint8_t *obuf, ssize_t olen);

uint8_t * gzip_encode(const uint8_t *ibuf, ssize_t ilen, ssize_t *olength);
uint8_t * gzip_decode(const uint8_t *ibuf, ssize_t ilen, ssize_t *olength);

ssize_t gzip_encode_to(const uint8_t *ibuf, ssize_t ilen, uint8_t *obuf, ssize_t olen);
ssize_t gzip_decode_to(const uint8_t *ibuf, ssize_t ilen, uint8_t *obuf, ssize_t olen);

