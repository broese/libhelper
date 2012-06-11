#include "lh_image.h"
#include "lh_buffers.h"
#include "lh_files.h"
#include "lh_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PNG_DEBUG 3
#include <png.h>

lhimage * allocate_image(int width, int height) {
    ALLOC(lhimage,img);
    img->width = width;
    img->height = height;
    img->png_transform = PNG_TRANSFORM_INVERT_ALPHA|PNG_TRANSFORM_BGR;
    ALLOCNE(uint32_t,img->data,width*height);
    return img;
}

lhimage * attach_image(int width, int height, uint32_t *data) {
    ALLOC(lhimage,img);
    img->width = width;
    img->height = height;
    img->png_transform = PNG_TRANSFORM_INVERT_ALPHA|PNG_TRANSFORM_BGR;
    img->data = data;
    return img;
}

void destroy_image(lhimage * img) {
    if (img) {
        if (img->data) free(img->data);
        free(img);
    }
}

////////////////////////////////////////////////////////////////////////////////

typedef struct {
    unsigned char * buffer;
    ssize_t size;
    ssize_t offset; // only used for reading
} pngbuf;

#define PNGGRAN 1024

static void pngio_read(png_structp png, png_bytep data, png_size_t length) {
    pngbuf * buf = (pngbuf *) png_get_io_ptr(png);

#if 0
    printf("\n--------------------\n"
           "FR: %08x %d\n"
           "TO: %08x %d\n",
           (uint32_t)buf->buffer, buf->offset,
           (uint32_t)data,length);
#endif

    if (buf->offset + length > buf->size)
        length = buf->size - buf->offset;
    memcpy(data, buf->buffer + buf->offset, length);

    //hexdump(data,length);

    buf->offset += length;
}

static void pngio_write(png_structp png, png_bytep data, png_size_t length) {
    pngbuf * buf = (pngbuf *) png_get_io_ptr(png);
    ssize_t offset = buf->size;
    BUFFER_ADDG(buf->buffer, buf->size, length, PNGGRAN);
    memcpy(buf->buffer + offset, data, length);
}

unsigned char * export_png(lhimage *img, ssize_t *osize) {
    pngbuf buffer;

    png_structp png = NULL;
    png_infop pngi  = NULL;


    // init PNG data structures
    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png)
        LH_ERROR(NULL,"png_create_write_struct failed\n");

    pngi = png_create_info_struct(png);
    if (!pngi) {
        png_destroy_write_struct(&png, &pngi);
        LH_ERROR(NULL,"png_create_info_struct failed\n");
    }


    // set our custom write function
    BUFFER_ALLOCG(buffer.buffer, buffer.size, 0, PNGGRAN);
    buffer.offset = 0;

    if (setjmp(png_jmpbuf(png))) {
        free(buffer.buffer);
        png_destroy_write_struct(&png, &pngi);
        LH_ERROR(NULL,"png_set_write_fn failed");
    }
    
    png_set_write_fn(png, &buffer, pngio_write, NULL);


    // write header
    if (setjmp(png_jmpbuf(png))) {
        free(buffer.buffer);
        png_destroy_write_struct(&png, &pngi);
        LH_ERROR(NULL,"png_set_IHDR failed");
    }

    png_set_IHDR(png, pngi, img->width, img->height,
                 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, pngi);


    // init row pointers
    ALLOCN(uint8_t *,rows,img->height);
    int i;
    for (i=0; i<img->height; i++)
        rows[i] = (uint8_t *)(img->data+i*img->width);
    png_set_rows(png, pngi, rows);


    // high level PNG write
    if (setjmp(png_jmpbuf(png))) {
        free(buffer.buffer);
        free(rows);
        png_destroy_write_struct(&png, &pngi);
        LH_ERROR(NULL,"png_write_png failed");
    }
    png_write_png(png, pngi, img->png_transform, NULL);

    free(rows);
    png_destroy_write_struct(&png, &pngi);

    *osize = buffer.size;
    return buffer.buffer;
}

ssize_t export_png_file(lhimage *img, const char *path) {
    ssize_t size;
    unsigned char * data = export_png(img, &size);
    if (!data) return -1;

    int result = write_file(path, data, size);
    free(data);

    return (result<0)?-1:size;
}

lhimage * import_png(unsigned char *data, ssize_t length) {
    pngbuf buffer;

    png_structp png = NULL;
    png_infop pngi  = NULL;
    png_infop pnge  = NULL;

    // check PNG signature
    if (png_sig_cmp(data, 0, 8))
        LH_ERROR(NULL,"Wrong PNG signature");


    // init PNG data structures
    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png)
        LH_ERROR(NULL,"png_create_write_struct failed\n");

    pngi = png_create_info_struct(png);
    if (!pngi) {
        png_destroy_read_struct(&png, &pngi, &pnge);
        LH_ERROR(NULL,"png_create_info_struct failed\n");
    }

#if 0
    pnge = png_create_info_struct(png);
    if (!pnge) {
        png_destroy_read_struct(&png, &pngi, &pnge);
        LH_ERROR(NULL,"png_create_info_struct failed\n");
    }
#endif


    // set our custom read function
    buffer.buffer = data;
    buffer.size   = length;
    buffer.offset = 0;

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &pngi, &pnge);
        LH_ERROR(NULL,"png_set_read_fn failed");
    }
    
    buffer.offset = 8;
    png_set_sig_bytes(png, 8);

    png_set_read_fn(png, &buffer, pngio_read);

    png_read_info(png, pngi);

    int width = png_get_image_width(png, pngi);
    int height = png_get_image_height(png, pngi);
    int color_type = png_get_color_type(png, pngi);
    int bit_depth = png_get_bit_depth(png, pngi);

    printf("W:%d H:%d C:%d D:%d\n",width, height, color_type, bit_depth);


    //FIXME: libpng bug?
    buffer.offset -= 8;


    // allocate image structure
    lhimage *img = allocate_image(width, height);


    // init row pointers
    ALLOCN(uint8_t *,rows,img->height);
    int i;
    for (i=0; i<img->height; i++)
        rows[i] = (uint8_t *)(img->data+i*img->width);
    png_set_rows(png, pngi, rows);

    // read image
    if (setjmp(png_jmpbuf(png))) {
        free(rows);
        destroy_image(img);
        png_destroy_read_struct(&png, &pngi, &pnge);
        LH_ERROR(NULL,"png_read_png failed");
    }
    //png_read_png(png, pngi, 0, NULL);
    png_read_png(png, pngi, img->png_transform, NULL);

    free(rows);
    png_destroy_read_struct(&png, &pngi, &pnge);

    return img;
}

lhimage * import_png_file(const char *path) {
    ssize_t size;
    unsigned char * data = read_file(path, &size);
    if (!data) return NULL;

    lhimage *img = import_png(data, size);
    free(data);
    return img;
}


