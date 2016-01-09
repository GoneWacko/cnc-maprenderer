#include <pnglite.h>
#include "output.h"
#include "map.h"
#include "renderer.h"

void output_png(image_t *img, const gchar *filename) {
    png_t p;
    png_init(0, 0);

    png_open_file_write(&p, filename);
    png_set_data(&p, img->width, img->height, 8, 2, (unsigned char *)img->data);
    png_close_file(&p);
}
