#ifndef MHENLO_RENDERER_H_INCLUDED
#define MHENLO_RENDERER_H_INCLUDED 

#include <glib.h>

#define TILE_WIDTH 24
#define TILE_HEIGHT 24

struct map;
typedef struct map map_t;

struct palette {
    guint8 colours[3*256];
};
extern struct palette palettes[3];

void load_tilemap();
void close_tilemap();

struct image {
    gint width;
    gint height;
    gchar *data;
};
typedef struct image image_t;

image_t *render_map(map_t *map);

#endif /* MHENLO_RENDERER_H_INCLUDED */
