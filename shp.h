#ifndef MHENLO_SHP_H_INCLUDED
#define MHENLO_SHP_H_INCLUDED 

#include <glib.h>

struct vfs_file;
typedef struct vfs_file vfs_file_t;

struct shp_offset;

struct shp {
    guint16 num_images;
    guint16 width;
    guint16 height;
    struct shp_offset *offsets;
    vfs_file_t *fp;
    GHashTable *refindex;
};
typedef struct shp shp_t;

shp_t *shp_init(vfs_file_t *fp);
void shp_free(shp_t *shp);

void shp_decompress(gchar *dest, shp_t *shp, gint index);

#endif /* MHENLO_SHP_H_INCLUDED */
