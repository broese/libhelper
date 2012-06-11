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
    int width;
    int height;
    uint32_t *data;
    int png_transform;
} lhimage;

////////////////////////////////////////////////////////////////////////////////

lhimage * allocate_image(int width, int height);
lhimage * attach_image(int width, int height, uint32_t *data);
void destroy_image(lhimage * img);

unsigned char * export_png(lhimage *img, ssize_t *osize);
ssize_t export_png_file(lhimage *img, const char *path);

lhimage * import_png(unsigned char *data, ssize_t length);
lhimage * import_png_file(const char *path);

