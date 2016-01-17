/*
 Authors:
 Copyright 2012-2015 by Eduard Broese <ed.broese@gmx.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version
 2 of the License, or (at your option) any later version.
*/

#include "lh_image.h"
#include "lh_buffers.h"
#include "lh_files.h"
#include "lh_debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PNG_DEBUG 3
#include <png.h>

lhimage * allocate_image(int32_t width, int32_t height, int32_t stride) {
    if (stride<=0) stride=width;

    lh_create_obj(lhimage,img);
    img->width  = width;
    img->height = height;
    img->stride = stride;

    lh_alloc_num(img->data,stride*height);

    return img;
}

lhimage * attach_image(int32_t width, int32_t height, uint32_t *data, int32_t stride) {
    lh_create_obj(lhimage,img);
    img->width = width;
    img->height = height;
    img->stride = stride;
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

//FIXME: implement more efficient copying
void resize_image(lhimage *img, int32_t newwidth, int32_t newheight,
                  int32_t offx, int32_t offy, uint32_t bgcolor, int32_t newstride) {
    if (newstride<=0) newstride=newwidth;

    // allocate the new image buffer
    uint32_t size = newstride*newheight;
    lh_create_num(uint32_t,newdata,size);

    // fill it with the background color
    int i;
    for (i=0; i<size; i++) newdata[i] = bgcolor;

    // from which XY position on the old image we may start
    int xmin=0; if (offx<0) xmin=-offx;
    int ymin=0; if (offy<0) ymin=-offy;

    // which is the maximum position on the original picture we can use
    int xmax=img->width;  if (xmax+offx > newwidth)  xmax = newwidth-offx;
    int ymax=img->height; if (ymax+offy > newheight) ymax = newheight-offy;

    int llen = (xmax-xmin)*sizeof(uint32_t);
    if (llen<=0) return;
    // this would mean the original picture was shifted beyond the edge of the new one

    int y;
    for(y=ymin; y<ymax; y++) {
        uint32_t *from = img->data+y*img->stride+xmin;
        uint32_t *to   = newdata+(y+offy)*newstride+(xmin+offx);
        memcpy(to, from, llen);
    }

    free(img->data);
    img->data = newdata;
    img->width = newwidth;
    img->height = newheight;
    img->stride = newstride;
}

////////////////////////////////////////////////////////////////////////////////

typedef struct {
    unsigned char * buffer;
    ssize_t size;
    ssize_t offset; // only used for reading
} pngbuf;

#define PNGGRAN 4096

#define PNGTRANS_DEFAULT_EXPORT (PNG_TRANSFORM_INVERT_ALPHA|PNG_TRANSFORM_BGR)
#define PNGTRANS_RGBA_IMPORT    PNGTRANS_DEFAULT_EXPORT
#define PNGTRANS_RGB_IMPORT

static void pngio_read(png_structp png, png_bytep data, png_size_t length) {
    pngbuf * buf = (pngbuf *) png_get_io_ptr(png);

    if (buf->offset + length > buf->size)
        length = buf->size - buf->offset;
    memcpy(data, buf->buffer + buf->offset, length);

    buf->offset += length;
}

static void pngio_write(png_structp png, png_bytep data, png_size_t length) {
    pngbuf * buf = (pngbuf *) png_get_io_ptr(png);
    ssize_t offset = buf->size;
    lh_arr_add(buf->buffer, buf->size, PNGGRAN, length);
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
    lh_arr_init(buffer.buffer, buffer.size);

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
    lh_create_num(uint8_t *,rows,img->height);
    int i;
    for (i=0; i<img->height; i++)
        rows[i] = (uint8_t *)(img->data+i*img->stride);
    png_set_rows(png, pngi, rows);


    // high level PNG write
    if (setjmp(png_jmpbuf(png))) {
        free(buffer.buffer);
        free(rows);
        png_destroy_write_struct(&png, &pngi);
        LH_ERROR(NULL,"png_write_png failed");
    }
    png_write_png(png, pngi, PNGTRANS_DEFAULT_EXPORT, NULL);

    free(rows);
    png_destroy_write_struct(&png, &pngi);

    *osize = buffer.size;
    return buffer.buffer;
}

ssize_t export_png_file(lhimage *img, const char *path) {
    ssize_t size;
    unsigned char * data = export_png(img, &size);
    if (!data) return -1;

    ssize_t wbytes = lh_save(path, data, size);
    free(data);

    return wbytes;
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

    switch (color_type) {
        case PNG_COLOR_TYPE_PALETTE:
            png_set_palette_to_rgb(png);
            //FIXME: -> RGBA
            break;
        case PNG_COLOR_TYPE_RGB:
            png_set_filler(png, 0x00, PNG_FILLER_AFTER);
            break;
    }


    //FIXME: libpng bug?
    buffer.offset -= 8;


    // allocate image structure
    lhimage *img = allocate_image(width, height, -1);


    // init row pointers
    lh_create_num(uint8_t *,rows,img->height);
    int i;
    for (i=0; i<img->height; i++)
        rows[i] = (uint8_t *)(img->data+i*img->stride);
    png_set_rows(png, pngi, rows);

    // read image
    if (setjmp(png_jmpbuf(png))) {
        free(rows);
        destroy_image(img);
        png_destroy_read_struct(&png, &pngi, &pnge);
        LH_ERROR(NULL,"png_read_png failed");
    }
    //png_read_png(png, pngi, 0, NULL);
    png_read_png(png, pngi, PNGTRANS_DEFAULT_EXPORT, NULL);

    free(rows);
    png_destroy_read_struct(&png, &pngi, &pnge);

    return img;
}

lhimage * import_png_file(const char *path) {
    unsigned char * data;
    ssize_t size = lh_load_alloc(path, &data);
    if (size<0) return NULL;

    lhimage *img = import_png(data, size);
    free(data);
    return img;
}


