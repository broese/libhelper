#pragma once

#include <stdint.h>
#include <stdlib.h>

/*
lh_image : image drawing, manipulations, import and export

Note that images are always interpreted as non-strided and have ARGB pixel
format in the host native byte-order, i.e.

pixelValue = (alpha<<24)+(r<<16)+(g<<8)+(b)

*/

typedef struct {
    int32_t width;
    int32_t height;
    int32_t stride;
    uint32_t *data;
} lhimage;

////////////////////////////////////////////////////////////////////////////////

lhimage * allocate_image(int32_t width, int32_t height, int32_t stride);
lhimage * attach_image(int32_t width, int32_t height, uint32_t *data, int32_t stride);
void destroy_image(lhimage * img);

unsigned char * export_png(lhimage *img, ssize_t *osize);
ssize_t export_png_file(lhimage *img, const char *path);

lhimage * import_png(unsigned char *data, ssize_t length);
lhimage * import_png_file(const char *path);

void resize_image(lhimage *img, int32_t newwidth, int32_t newheight,
                  int32_t offx, int32_t offy, uint32_t bgcolor, int32_t newstride);

////////////////////////////////////////////////////////////////////////////////

#define IMGDOT(img,x,y) img->data[(x)+img->width*(y)]
