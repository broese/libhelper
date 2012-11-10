#include "lh_debug.h"
#include "lh_compress.h"
#include "lh_buffers.h"
#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#define ALLOCGRAN 4096

#define FORMAT_ZLIB 0
#define FORMAT_GZIP 1
#define FORMAT_AUTO 2

#define OP_ENCODE   0
#define OP_DECODE   1

#define AM_STATIC   0
// do not reallocate buffers
// if there is no space, abort with error
// olen contains buffer size on enter
// return value is the length (olen is not modified)

#define AM_DYNAMIC  1
// realloc buffers if needed
// olen contains maximum allowed buffer size 

static uint8_t * zlib_internal(int op, int format, int complevel, int alloc_mode,
                             const uint8_t *ibuf, ssize_t ilen, uint8_t **obuf, ssize_t *olen,
                             ptrdiff_t offset) {
    
    // set up zlib state parameters
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree  = Z_NULL;
    zs.opaque = Z_NULL;

    zs.next_in  = ibuf;
    zs.avail_in = ilen;


    // initialize zlib operation
    int result;
    if (op == OP_ENCODE) {
        switch (format) {
        case FORMAT_ZLIB:
            result = deflateInit(&zs, complevel);
            break;
        case FORMAT_GZIP:
            result = deflateInit2(&zs, complevel, Z_DEFLATED, 16+MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
            break;
        default:
            LH_ERROR(NULL,"zlib_internal: unsupported output format %d\n",format);
            break;
        }
        if (result != Z_OK)
            LH_ERROR(NULL,"deflateInit failed, error %d\n",result);
    }
    else {
        switch (format) {
        case FORMAT_ZLIB:
            result = inflateInit(&zs);
            break;
        case FORMAT_GZIP:
            result = inflateInit2(&zs, 16+MAX_WBITS);
            break;
        case FORMAT_AUTO:
            result = inflateInit2(&zs, 32+MAX_WBITS);
            break;
        default:
            LH_ERROR(NULL,"zlib_encode_internal: unsupported output format %d\n",format);
            break;
        }
        if (result != Z_OK)
            LH_ERROR(NULL,"inflateInit failed, error %d\n",result);
    }

    // check if we need to pre-allocate the output buffer
    if (*olen-offset <= 0) {
        if (alloc_mode == AM_STATIC) {
            LH_ERROR(NULL,"unsufficient output buffer size %zd\n", *olen);
        }
        else {
            ARRAY_ALLOCG(*obuf, *olen, LH_COMPRESS_ALLOCGRAN+offset, LH_COMPRESS_ALLOCGRAN);
        }
    }

    zs.next_out = *obuf+offset;
    zs.avail_out = *olen-offset;

    do {
        if (op == OP_ENCODE) {
            result = deflate(&zs, Z_FINISH); //Z_FINISH since all input data is available
            
            if (result == Z_STREAM_ERROR) {
                deflateEnd(&zs);
                LH_ERROR(NULL,"zlib reported Z_STREAM_ERROR\n");
                //TODO: user should free the buffer
            }
        }
        else {
            result = inflate(&zs, Z_FINISH); //Z_FINISH since all input data is available
            
            if (result == Z_STREAM_ERROR) {
                inflateEnd(&zs);
                LH_ERROR(NULL,"zlib reported Z_STREAM_ERROR\n");
                //TODO: user should free the buffer
            }
        }            

        ssize_t outsize = zs.next_out - *obuf; // current size of data in the buffer
        if (result == Z_BUF_ERROR) {
            if (alloc_mode == AM_STATIC)
                LH_ERROR(NULL,"unsufficient output buffer size %zd\n", *olen);

            ARRAY_ADDG(*obuf, *olen, LH_COMPRESS_ALLOCGRAN, LH_COMPRESS_ALLOCGRAN);
            zs.next_out = *obuf + outsize;
            zs.avail_out = *olen - outsize;
        }
    } while(result != Z_STREAM_END);

    if (op == OP_ENCODE)
        deflateEnd(&zs);
    else
       inflateEnd(&zs);

    return zs.next_out;
}

ssize_t zlib_encode_to(const uint8_t *ibuf, ssize_t ilen, uint8_t *obuf, ssize_t olen) {
    uint8_t *wp = zlib_internal(OP_ENCODE, FORMAT_ZLIB, Z_DEFAULT_COMPRESSION, AM_STATIC,
                                ibuf, ilen, &obuf, &olen, 0);
    return wp ? wp - obuf : -1;
}

ssize_t zlib_decode_to(const uint8_t *ibuf, ssize_t ilen, uint8_t *obuf, ssize_t olen) {
    uint8_t *wp = zlib_internal(OP_DECODE, FORMAT_ZLIB, Z_DEFAULT_COMPRESSION, AM_STATIC,
                                ibuf, ilen, &obuf, &olen, 0);
    return wp ? wp - obuf : -1;
}


uint8_t * zlib_encode(const uint8_t *ibuf, ssize_t ilen, ssize_t *olength) {
    BUFFER(obuf,olen);
    uint8_t *wp = zlib_internal(OP_ENCODE, FORMAT_ZLIB, Z_DEFAULT_COMPRESSION, AM_DYNAMIC,
                                ibuf, ilen, &obuf, &olen, 0);
    if (wp) {
        *olength = wp-obuf;
        return obuf;
    }
    else {
        if (obuf) free(obuf);
        *olength = 0;
        return NULL;
    }
}
uint8_t * zlib_decode(const uint8_t *ibuf, ssize_t ilen, ssize_t *olength) {
    BUFFER(obuf,olen);
    uint8_t *wp = zlib_internal(OP_DECODE, FORMAT_ZLIB, Z_DEFAULT_COMPRESSION, AM_DYNAMIC,
                                ibuf, ilen, &obuf, &olen, 0);
    if (wp) {
        *olength = wp-obuf;
        return obuf;
    }
    else {
        if (obuf) free(obuf);
        *olength = 0;
        return NULL;
    }
}

ssize_t gzip_encode_to(const uint8_t *ibuf, ssize_t ilen, uint8_t *obuf, ssize_t olen) {
    uint8_t *wp = zlib_internal(OP_ENCODE, FORMAT_GZIP, Z_DEFAULT_COMPRESSION, AM_STATIC,
                                ibuf, ilen, &obuf, &olen, 0);
    return wp ? wp - obuf : -1;
}

ssize_t gzip_decode_to(const uint8_t *ibuf, ssize_t ilen, uint8_t *obuf, ssize_t olen) {
    uint8_t *wp = zlib_internal(OP_DECODE, FORMAT_GZIP, Z_DEFAULT_COMPRESSION, AM_STATIC,
                                ibuf, ilen, &obuf, &olen, 0);
    return wp ? wp - obuf : -1;
}


uint8_t * gzip_encode(const uint8_t *ibuf, ssize_t ilen, ssize_t *olength) {
    BUFFER(obuf,olen);
    uint8_t *wp = zlib_internal(OP_ENCODE, FORMAT_GZIP, Z_DEFAULT_COMPRESSION, AM_DYNAMIC,
                                ibuf, ilen, &obuf, &olen, 0);
    if (wp) {
        *olength = wp-obuf;
        return obuf;
    }
    else {
        if (obuf) free(obuf);
        *olength = 0;
        return NULL;
    }
}
uint8_t * gzip_decode(const uint8_t *ibuf, ssize_t ilen, ssize_t *olength) {
    BUFFER(obuf,olen);
    uint8_t *wp = zlib_internal(OP_DECODE, FORMAT_GZIP, Z_DEFAULT_COMPRESSION, AM_DYNAMIC,
                                ibuf, ilen, &obuf, &olen, 0);
    if (wp) {
        *olength = wp-obuf;
        return obuf;
    }
    else {
        if (obuf) free(obuf);
        *olength = 0;
        return NULL;
    }
}



