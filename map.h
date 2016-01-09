#ifndef MHENLO_MAP_H_INCLUDED
#define MHENLO_MAP_H_INCLUDED 

#include <glib.h>
#include "ini/ini.h"

#define MAP_WIDTH 64
#define MAP_HEIGHT 64
#define XY_TO_COORD(x,y) (((y) * MAP_WIDTH) + (x))
#define COORD_TO_X(c) ((c) % MAP_WIDTH)
#define COORD_TO_Y(c) ((c) / MAP_WIDTH)

#define NEIGHBOUR_TOP 1
#define NEIGHBOUR_RIGHT (1 << 1)
#define NEIGHBOUR_BOTTOM (1 << 2)
#define NEIGHBOUR_LEFT (1 << 3)
// TODO Fix these
#define NEIGHBOUR_TOPLEFT 0
#define NEIGHBOUR_TOPRIGHT 0
#define NEIGHBOUR_BOTTOMRIGHT 0
#define NEIGHBOUR_BOTTOMLEFT 0

struct map {
    struct {
        guint8 height;
        guint8 width;
        guint8 x;
        guint8 y;
        gchar theater[10];
    } map;
    const gchar *bin;
    GSequence *entities;
    GSequence *terrain;
};
typedef struct map map_t;

enum House {
    HOUSE_GOODGUY,
    HOUSE_BADGUY,
    HOUSE_NEUTRAL,
    HOUSE_SPECIAL
    // Multi1-5
};

struct structure {
    gchar owner[8];
    gushort health;
    gushort rotation;
    //gchar trigger[8];
};
typedef struct structure structure_t;

struct overlay {
    guint8 neighbours;
};
typedef struct overlay overlay_t;

struct unit {
    gchar owner[8];
    gushort health;
    gushort rotation;
    // - Action
    // - Trigger
};
typedef struct unit unit_t;

enum EntityKind {
    ENTITY_OVERLAY,
    ENTITY_STRUCTURE,
    ENTITY_UNIT
};

struct terrain {
    gushort location;
    gchar type[9];
};
typedef struct terrain terrain_t;

struct entity {
    gushort location;
    gchar type[9];
    enum EntityKind kind;
    union {
        overlay_t overlay;
        structure_t structure;
        unit_t unit;
    } data;
};
typedef struct entity entity_t;

void map_parse(map_t *dest, ini_t *ini, const gchar bin[]);

#endif /* MHENLO_MAP_H_INCLUDED */
