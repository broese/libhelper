/*
 Authors:
 Copyright 2012-2015 by Eduard Broese <ed.broese@gmx.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version
 2 of the License, or (at your option) any later version.
*/

#pragma once

#include <stdlib.h>
#include <stdint.h>

/*
lh_compress - support for compression/decompression of data
*/

#define LH_COMPRESS_ALLOCGRAN 4096

////////////////////////////////////////////////////////////////////////////////

uint8_t * lh_zlib_encode(const uint8_t *ibuf, ssize_t ilen, ssize_t *olength);
uint8_t * lh_zlib_decode(const uint8_t *ibuf, ssize_t ilen, ssize_t *olength);

ssize_t lh_zlib_encode_to(const uint8_t *ibuf, ssize_t ilen, uint8_t *obuf, ssize_t olen);
ssize_t lh_zlib_decode_to(const uint8_t *ibuf, ssize_t ilen, uint8_t *obuf, ssize_t olen);

uint8_t * lh_gzip_encode(const uint8_t *ibuf, ssize_t ilen, ssize_t *olength);
uint8_t * lh_gzip_decode(const uint8_t *ibuf, ssize_t ilen, ssize_t *olength);

ssize_t lh_gzip_encode_to(const uint8_t *ibuf, ssize_t ilen, uint8_t *obuf, ssize_t olen);
ssize_t lh_gzip_decode_to(const uint8_t *ibuf, ssize_t ilen, uint8_t *obuf, ssize_t olen);

////////////////////////////////////////////////////////////////////////////////

#ifdef LH_DECLARE_SHORT_NAMES

#define zlib_encode             lh_zlib_encode
#define zlib_decode             lh_zlib_decode
#define zlib_encode_to          lh_zlib_encode_to
#define zlib_decode_to          lh_zlib_decode_to

#define gzip_encode             lh_gzip_encode
#define gzip_decode             lh_gzip_decode
#define gzip_encode_to          lh_gzip_encode_to
#define gzip_decode_to          lh_gzip_decode_to

#endif

