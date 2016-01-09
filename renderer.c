#include "renderer.h"
#include "map.h"
#include "vfs/vfs.h"
#include "shp.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

struct palette palettes[3];
GTree *templatecache;
GTree *tilemap;
gchar *tilemapbuf;

struct template {
    // TODO can be rejiggered so that it's one block of memory
    guint16 tile_count;
    guint8 *tile_index;
    gchar *tilebuf;
};
typedef struct template template_t;

const guint8 badguy_structure_remap[16] = {
    0x7f, 0x7e, 0x7d, 0x7c, 0x7a, 0x2e, 0x78, 0x2f, 0x7d, 0x7c, 0x7b, 0x7a, 0x2a, 0x79, 0x78, 0x78
};
const guint8 badguy_unit_remap[16] = {
    0xa1, 0xc8, 0xc9, 0xca, 0xcc, 0xcd, 0xce, 0x0c, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0x73, 0xc6, 0x72
};

gint byte_compare(gconstpointer v1, gconstpointer v2, gpointer data) {
    if (*(const guint8 *) v1 < *(const guint8*) v2) return -1;
    else return *(const guint8 *) v1 - *(const guint8*) v2;
}

void load_tilemap() {
    vfs_file_t *fp = vfs_file_open("tilemap.dat");
    gsize len = vfs_file_size(fp);
    tilemapbuf = g_malloc(len);
    vfs_file_read(tilemapbuf, len, fp);
    vfs_file_close(fp);

    tilemap = g_tree_new_with_data(byte_compare, NULL);

    guint8 num = *(guint8*)&tilemapbuf[0];
    guint8 i = 0;
    gchar *cur = &tilemapbuf[1];
    while (i < num) {
        guint8 *index = (guint8*)cur++;
        gchar *name = cur++;
        while (*cur++ != 0);
        ++i;
        g_tree_insert(tilemap, index, name);
    }
}

void close_tilemap() {
    g_tree_destroy(tilemap);
    g_free(tilemapbuf);
}

void load_palettes(gchar *theater) {
    gchar filename[13];
    sprintf(filename, "%.8s.pal", theater);
    vfs_file_t *fp = vfs_file_open(filename);
    for (int i = 0; i < 256; ++i) {
        vfs_file_read((gchar*)&palettes[0].colours[i*3], 3, fp);
        palettes[0].colours[i*3] <<= 2;
        palettes[0].colours[i*3+1] <<= 2;
        palettes[0].colours[i*3+2] <<= 2;
    }
    vfs_file_close(fp);

    // Copy palette[0] and remap 176-192 to BadGuy colours
    memcpy(&palettes[1].colours, &palettes[0].colours[0*3], 256*3);
    for (int i = 0; i < 16; ++i) {
        palettes[1].colours[(i+176)*3] = palettes[0].colours[badguy_structure_remap[i] * 3];
        palettes[1].colours[(i+176)*3 + 1] = palettes[0].colours[badguy_structure_remap[i] * 3 + 1];
        palettes[1].colours[(i+176)*3 + 2] = palettes[0].colours[badguy_structure_remap[i] * 3 + 2];
    }

    // And again for BadGuy unit colours
    memcpy(&palettes[2].colours, &palettes[0].colours[0*3], 256*3);
    for (int i = 0; i < 16; ++i) {
        palettes[2].colours[(i+176)*3] = palettes[0].colours[badguy_unit_remap[i] * 3];
        palettes[2].colours[(i+176)*3 + 1] = palettes[0].colours[badguy_unit_remap[i] * 3 + 1];
        palettes[2].colours[(i+176)*3 + 2] = palettes[0].colours[badguy_unit_remap[i] * 3 + 2];
    }
}

const gchar missing_tile[TILE_WIDTH * TILE_HEIGHT] = { 0 }; // Red square

const gchar *get_tile(guint8 filenum, guint8 spritenum, gchar *theater) {
    template_t *template = g_tree_lookup(templatecache, &filenum);
    if (!template) {
        guint8 *key;
        gchar *templatename;
        g_tree_lookup_extended(tilemap, &filenum, (gpointer *)&key, (gpointer *)&templatename);
        gchar filename[13];
        sprintf(filename, "%.8s.%.3s", templatename, theater);
        vfs_file_t *fp = vfs_file_open(filename);
        if (!fp) {
            // Annoyingly, some maps have invalid tile/theater
            // combinations in them... (e.g. scg03ea)
            return missing_tile;
        }
        template = g_new(template_t, 1);

        vfs_file_seek(fp, 4, VFS_SEEK_SET);
        vfs_file_read((gchar *)&template->tile_count, 2, fp); // TODO Endianness?
        guint32 image_offset;
        vfs_file_seek(fp, 12, VFS_SEEK_SET);
        vfs_file_read((gchar *)&image_offset, 4, fp); // TODO Endianness?
        guint32 image_index_offset;
        guint32 tile_index_offset;
        vfs_file_seek(fp, 24, VFS_SEEK_SET);
        vfs_file_read((gchar *)&image_index_offset, 4, fp); // TODO Endianness?
        vfs_file_read((gchar *)&tile_index_offset, 4, fp); // TODO Endianness?
        vfs_file_seek(fp, tile_index_offset, VFS_SEEK_SET);

        template->tile_index = g_malloc(tile_index_offset);
        vfs_file_read((gchar *)template->tile_index, template->tile_count, fp);

        guint16 tilebufsize = (image_index_offset - image_offset);
        template->tilebuf = g_malloc(tilebufsize);
        vfs_file_seek(fp, image_offset, VFS_SEEK_SET);
        vfs_file_read(template->tilebuf, tilebufsize, fp);
        vfs_file_close(fp);
    
#ifdef DRAW_DEBUG_GRID
    for (unsigned char i = 0; i < 24; ++i) {
        template->tilebuf[i] = 0xDA;
        template->tilebuf[i*24] = 0xDA;
        template->tilebuf[i*24+23] = 0xDB;
        template->tilebuf[551+i] = 0xDB;
    }
#endif

        g_tree_insert(templatecache, key, template);
    }
    gchar *tile = template->tilebuf + (template->tile_index[spritenum] * TILE_WIDTH * TILE_HEIGHT);
    return tile;
}

void free_template(gpointer data) {
    template_t *template = (template_t *)data;
    g_free(template->tilebuf);
    g_free(template->tile_index);
    g_free(template);
}

void do_copy_sprite(gchar *imgbuf, const gchar *sprite, gushort px, gushort py, guint16 width, guint16 height, struct palette *pal);

void copy_sprite(gchar *imgbuf, const gchar *sprite, guint8 x, guint8 y, guint16 width, guint16 height, struct palette *pal) {
    gushort px = x * TILE_WIDTH;
    gushort py = y * TILE_HEIGHT;
    do_copy_sprite(imgbuf, sprite, px, py, width, height, pal);
}

void copy_sprite_centered(gchar *imgbuf, const gchar *sprite, guint8 x, guint8 y, guint16 width, guint16 height, struct palette *pal) { 
    // TODO Clamp images if they should be drawn partially(?)
    gint px = x * TILE_WIDTH - (width / 2) + (TILE_WIDTH / 2);
    gint py = y * TILE_HEIGHT - (height / 2) + (TILE_HEIGHT / 2);
    do_copy_sprite(imgbuf, sprite, px, py, width, height, pal);
}

void do_copy_sprite(gchar *imgbuf, const gchar *sprite, gushort px, gushort py, guint16 width, guint16 height, struct palette *pal) {
    const gchar *sp = sprite;
    gchar *dp = imgbuf + (((py * TILE_WIDTH * MAP_WIDTH) + px) * 3);
    for (guint16 sy = 0; sy < height; ++sy) {
        for (guint16 sx = 0; sx < width; ++sx) {
            guint8 colour = sp[sx];
            if (colour == 0x00 || colour == 0x04) continue; // TODO 0x04 = Shadow
            memcpy(&dp[sx*3], &pal->colours[colour*3], 3);
        }
        sp += width;
        dp += (TILE_WIDTH * MAP_WIDTH * 3);
    }
}

void render_templates(gchar *imgbuf, map_t *map) {
    int i = 0;
    int x = 0;
    int y = 0;
    if (!templatecache) {
        templatecache = g_tree_new_full(byte_compare, NULL, NULL, free_template);
    }
    while (i < MAP_WIDTH * MAP_HEIGHT * 2) {
        guint8 filenum = map->bin[i++];
        guint8 spritenum = map->bin[i++];
        if (filenum == 0xFF) {
            filenum = 0x00;
        }
        const gchar *tile = get_tile(filenum, spritenum, map->map.theater);

        copy_sprite(imgbuf, tile, x, y, TILE_WIDTH, TILE_HEIGHT, &palettes[0]);

        x = x + 1;
        if (x == MAP_WIDTH) {
            y = y + 1;
            x = 0;
        }
    }
}

struct render_data {
    gchar *imgbuf;
    map_t *map;
};

gboolean render_shp(gchar *dest, const map_t *map, const gchar *name, gint index, guint16 location, struct palette *pal, gboolean center) {
    gchar filename[13];
    guint8 x = COORD_TO_X(location);
    guint8 y = COORD_TO_Y(location);
    sprintf(filename, "%.8s.SHP", name);
    vfs_file_t *fp = vfs_file_open(filename);
    if (!fp) {
        sprintf(filename, "%.8s.%.3s", name, map->map.theater);
        fp = vfs_file_open(filename);
        if (!fp) {
            fprintf(stderr, "What the... %s can't be found\n", name);
            return FALSE;
        }
    }
    //fprintf(stderr, "Rendering SHP: %s\n", filename);
    shp_t *shp = shp_init(fp);
    gchar *buf = g_malloc(shp->width * shp->height);
    shp_decompress(buf, shp, index);
    if (center) {
        copy_sprite_centered(dest, buf, x, y, shp->width, shp->height, pal);
    } else {
        copy_sprite(dest, buf, x, y, shp->width, shp->height, pal);
    }
    g_free(buf);
    shp_free(shp);
    return TRUE;
}

void render_overlay(gchar *dest, const entity_t *e, const map_t *map) {
    gint idx = 0;
    if (strncmp(e->type, "SBAG", 4) == 0 || strncmp(e->type, "BRIK", 4) == 0 || strncmp(e->type, "WOOD", 4) == 0 || strncmp(e->type, "CYCL", 4) == 0 || strncmp(e->type, "BARB", 4) == 0) {
        idx = e->data.overlay.neighbours;
    } else if (e->type[0] == 'T' && e->type[1] == 'I' && isdigit(e->type[2])) {
        idx = 8;    // TODO Research alternatives
    };

    render_shp(dest, map, e->type, idx, e->location, &palettes[0], FALSE);
}

void render_structure(gchar *dest, const entity_t *e, const map_t *map) {
    // TODO Health, Rotation
    struct palette *pal;
    // TODO use the HOUSE enum so this performs better
    if (strncmp(e->data.structure.owner, "BadGuy", 6) == 0) {
        pal = &palettes[1];    
    } else {
        pal = &palettes[0]; 
    }

    render_shp(dest, map, e->type, 0, e->location, pal, FALSE);
    // Maybe make this something a little more extendable
    if (strncmp(e->type, "WEAP", 4) == 0) {
        render_shp(dest, map, "WEAP2", 0, e->location, pal, FALSE);
    }
}

void render_unit(gchar *dest, const entity_t *e, const map_t *map) {
    // TODO Health, Rotation, special cases (WEAP comes to mind)
    struct palette *pal;
    // TODO use the HOUSE enum so this performs better
    if (strncmp(e->data.unit.owner, "BadGuy", 6) == 0) {
        // TODO Account for HARV, MCV (and others?) that should be rendered with palette[1]
        pal = &palettes[2];
    } else {
        pal = &palettes[0]; 
    }

    render_shp(dest, map, e->type, 0, e->location, pal, TRUE);
    // TODO Turrets
    // TODO Center units with sprites larger than 24 * 24 (i.e. MCV)
}

void render_entity(gpointer data, gpointer user_data) {
    struct render_data *rdata = (struct render_data *)user_data;
    switch (((entity_t *)data)->kind) {
        case ENTITY_OVERLAY:
            render_overlay(rdata->imgbuf, (entity_t *)data, rdata->map);
            break;
        case ENTITY_STRUCTURE:
            render_structure(rdata->imgbuf, (entity_t *)data, rdata->map);
            break;
        case ENTITY_UNIT:
            render_unit(rdata->imgbuf, (entity_t *)data, rdata->map);
            break;
        default:
            break; // Don't render TERRAIN this pass
    }
}

void render_terrain(gpointer data, gpointer user_data) {
    struct render_data *rdata = (struct render_data *)user_data;
    terrain_t *t = (terrain_t *)data;
    render_shp(rdata->imgbuf, rdata->map, t->type, 0, t->location, &palettes[0], FALSE);
}

image_t *render_map(map_t *map) {
    struct render_data rdata;
    image_t *img = g_new(image_t, 1);
    img->data = g_malloc(MAP_WIDTH * MAP_HEIGHT * TILE_WIDTH * TILE_HEIGHT * 3);
    img->width = MAP_WIDTH * TILE_WIDTH;
    img->height = MAP_HEIGHT * TILE_HEIGHT;

    rdata.map = map;
    rdata.imgbuf = img->data;

    load_palettes(map->map.theater);

    render_templates(img->data, map);
    // TODO BIBs (sandy patches in front of buildings)
    g_sequence_foreach(map->entities, render_entity, &rdata);
    g_sequence_foreach(map->terrain, render_terrain, &rdata);
    // TODO Shadows
    
    return img;
}
