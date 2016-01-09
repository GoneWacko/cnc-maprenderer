#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "vfs/vfs.h"
#include "ini/ini.h"
#include "ini/parser.h"
#include "map.h"
#include "renderer.h"
#include "output.h"

#define BINSIZE 8192

enum {
    ERR_OK,
    ERR_MAPNAME_TOO_LONG,
    ERR_NONEXISTENT_MAP,
    ERR_INVALID_BINFILE
};

gchar *error_text[] = {
    "",
    "Map names cannot be longer than 8 characters (e.g. 'scg01ea')\n",
    "The requested map cannot be found\n",
    "The .bin file is invalid (wrong file size)\n",
};

struct {
    gchar *mapname;
} config;

void init() {
    vfs_init();
    vfs_add_archive("data");
    vfs_add_archive("data/GENERAL_GDI.MIX");
    vfs_add_archive("data/GENERAL_NOD.MIX");
    vfs_add_archive("data/CONQUER.MIX");

    load_tilemap();
};

void finish() {
    close_tilemap();
    vfs_close();
}

int configure(int argc, char *argv[]) {
    if (argc > 1) {
        if (strlen(argv[1]) > 8) {
            return ERR_MAPNAME_TOO_LONG;
        } else {
            config.mapname = argv[1];
        }
    } else {
        config.mapname = "scg01ea";
    }
    return ERR_OK;
}

int run(int argc, char *argv[]) {
    int error = configure(argc, argv);
    if (error) {
        fprintf(stderr, error_text[error]);
        return error;
    }

    printf("Loading map '%s'...\n", config.mapname);

    gchar filename[13];
    g_snprintf(filename, 13, "%s.ini", config.mapname);
    if (!vfs_file_exists(filename)) {
        fprintf(stderr, error_text[ERR_NONEXISTENT_MAP]);
        return ERR_NONEXISTENT_MAP;
    }
    vfs_file_t *inifile = vfs_file_open(filename);

    g_snprintf(filename, 13, "%s.bin", config.mapname);
    if (!vfs_file_exists(filename)) {
        fprintf(stderr, error_text[ERR_NONEXISTENT_MAP]);
        return ERR_NONEXISTENT_MAP;
    }
    vfs_file_t *binfile = vfs_file_open(filename);
    if (vfs_file_size(binfile) != BINSIZE) {
        fprintf(stderr, error_text[ERR_INVALID_BINFILE]);
        return ERR_INVALID_BINFILE;
    }

    guint32 inisize = vfs_file_size(inifile);
    gchar *inibuf = g_malloc(inisize);
    vfs_file_read(inibuf, inisize, inifile);
    vfs_file_close(inifile);

    gchar *binbuf = g_malloc(BINSIZE);
    vfs_file_read(binbuf, BINSIZE, binfile);
    vfs_file_close(binfile);

    printf("Parsing map information...\n");

    ini_t *ini = ini_parse(inibuf, inisize);
    g_free(inibuf);

    map_t map;
    map_parse(&map, ini, binbuf);

    gchar theatermix[18];
    sprintf(theatermix, "data/%.8s.MIX", map.map.theater);
    vfs_add_archive(theatermix);

    printf("Rendering map...\n");

    image_t *img = render_map(&map);

    gchar outfilename[13];
    sprintf(outfilename, "%.8s.png", config.mapname);
    printf("Writing PNG file...\n");

    output_png(img, outfilename);

    g_free(binbuf);
    ini_free(ini);

    return ERR_OK; 
}

int main(int argc, char *argv[]) {
    init();

    int result = run(argc, argv);
    
    finish();
    return result;
}
