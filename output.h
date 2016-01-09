#ifndef MHENLO_OUTPUT_H_INCLUDED
#define MHENLO_OUTPUT_H_INCLUDED 

#include <glib.h>

struct image;
typedef struct image image_t;

void output_png(image_t *img, const gchar *filename);

#endif /* MHENLO_OUTPUT_H_INCLUDED */
