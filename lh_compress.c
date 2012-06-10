#include "lh_debug.h"
#include "lh_compress.h"
#include "lh_buffers.h"
#include <zlib.h>
#include <stdlib.h>
#include <stdio.h>

#define ALLOCGRAN 4096

#define FORMAT_ZLIB 0
#define FORMAT_GZIP 1
#define FORMAT_AUTO 2

static unsigned char * zlib_encode_internal
(const unsigned char *data, ssize_t length, ssize_t *olength, int format, int complevel) {
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree  = Z_NULL;
    zs.opaque = Z_NULL;

    int result;

    switch (format) {
    case FORMAT_ZLIB:
        result = deflateInit(&zs, complevel);
        break;
    case FORMAT_GZIP:
        result = deflateInit2(&zs, complevel, Z_DEFLATED, 16+MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
        break;
    default:
        LH_ERROR(NULL,"zlib_encode_internal: unsupported output format %d\n",format);
        break;
    }
    if (result != Z_OK)
        LH_ERROR(NULL,"deflateInit failed, error %d\n",result);

    zs.next_in  = data;
    zs.avail_in = length;

    unsigned char *odata;
    BUFFER_ALLOCG(odata, *olength, ALLOCGRAN, ALLOCGRAN);
    zs.next_out = odata;
    zs.avail_out = *olength;

    do {
        printf("IN : %08p %d\n"
               "OUT: %08p %d\n",
               zs.next_in, zs.avail_in,
               zs.next_out, zs.avail_out);
               
        result = deflate(&zs, Z_FINISH); //Z_FINISH since all input data is available
        if (result == Z_STREAM_ERROR) {
            free(odata);
            LH_ERROR(NULL,"zlib reported Z_STREAM_ERROR\n");
        }

        ssize_t outsize = zs.next_out - odata;
        if (result == Z_BUF_ERROR) {
        //        if (zs.avail_out < ALLOCGRAN) {
            BUFFER_ADDG(odata, *olength, ALLOCGRAN, ALLOCGRAN);
            zs.next_out = odata + outsize;
            zs.avail_out = *olength - outsize;
        }
    } while(result != Z_STREAM_END);

    BUFFER_EXTENDG(odata, *olength, zs.total_out, ALLOCGRAN);
    return odata;
}

static unsigned char * zlib_decode_internal
(const unsigned char *data, ssize_t length, ssize_t *olength, int format) {
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree  = Z_NULL;
    zs.opaque = Z_NULL;
    zs.next_in  = data;
    zs.avail_in = length;

    int result;
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

    unsigned char *odata;
    BUFFER_ALLOCG(odata, *olength, ALLOCGRAN, ALLOCGRAN);
    zs.next_out = odata;
    zs.avail_out = *olength;

    do {
        result = inflate(&zs, Z_FINISH); //Z_FINISH since all input data is available
        printf("\n"
               "result = %d\n"
               "IN : %08p %d\n"
               "OUT: %08p %d\n",
               result,
               zs.next_in, zs.avail_in,
               zs.next_out, zs.avail_out);
               
        if (result == Z_STREAM_ERROR) {
            free(odata);
            LH_ERROR(NULL,"zlib reported Z_STREAM_ERROR\n");
        }
        if (result == Z_STREAM_ERROR) {
            free(odata);
            LH_ERROR(NULL,"Incorrect zlib data. Z_DATA_ERROR\n");
        }

        ssize_t outsize = zs.next_out - odata;
        if (result == Z_BUF_ERROR) {
        //        if (zs.avail_out < ALLOCGRAN) {
            BUFFER_ADDG(odata, *olength, ALLOCGRAN, ALLOCGRAN);
            zs.next_out = odata + outsize;
            zs.avail_out = *olength - outsize;
        }
    } while(result != Z_STREAM_END);

    BUFFER_EXTENDG(odata, *olength, zs.total_out, ALLOCGRAN);
    return odata;
}

unsigned char * zlib_encode(const unsigned char *data, ssize_t length, ssize_t *olength) {
    return zlib_encode_internal(data, length, olength, FORMAT_ZLIB, Z_DEFAULT_COMPRESSION);
}

unsigned char * gzip_encode(const unsigned char *data, ssize_t length, ssize_t *olength) {
    return zlib_encode_internal(data, length, olength, FORMAT_GZIP, Z_DEFAULT_COMPRESSION);
}

unsigned char * zlib_decode(const unsigned char *data, ssize_t length, ssize_t *olength) {
    return zlib_decode_internal(data, length, olength, FORMAT_ZLIB);
}

unsigned char * gzip_decode(const unsigned char *data, ssize_t length, ssize_t *olength) {
    return zlib_decode_internal(data, length, olength, FORMAT_GZIP);
}

