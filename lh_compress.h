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

#define zlib_encode(ibuf,ilen,olength)      lh_zlib_encode(ibuf,ilen,olength)
#define zlib_decode(ibuf,ilen,olength)      lh_zlib_decode(ibuf,ilen,olength)
#define zlib_encode_to(ibuf,ilen,obuf,olen) lh_zlib_encode_to(ibuf,ilen,obuf,olen)
#define zlib_decode_to(ibuf,ilen,obuf,olen) lh_zlib_decode_to(ibuf,ilen,obuf,olen)

#define gzip_encode(ibuf,ilen,olength)      lh_gzip_encode(ibuf,ilen,olength)
#define gzip_decode(ibuf,ilen,olength)      lh_gzip_decode(ibuf,ilen,olength)
#define gzip_encode_to(ibuf,ilen,obuf,olen) lh_gzip_encode_to(ibuf,ilen,obuf,olen)
#define gzip_decode_to(ibuf,ilen,obuf,olen) lh_gzip_decode_to(ibuf,ilen,obuf,olen)

#endif

