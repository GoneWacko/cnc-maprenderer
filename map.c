#include "map.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct csv_substring {
    const gchar *start;
    gsize len;
};

struct csv {
    const gchar *data;
    gsize count;
    struct csv_substring *substrings;
};
typedef struct csv csv_t;

gsize csv_parse(csv_t *csv) {
    guint count = 0;
    const gchar *c = csv->data;
    while (1) {
        if (*c == ',') {
            ++count;
        } else if (*c == '\0') {
            break;
        }
        ++c;
    }
    if (!count && c == csv->data) {
        return 0;
    }
    
    csv->count = count + 1;
    csv->substrings = g_new(struct csv_substring, csv->count);
    c = csv->data;
    count = 0;
    while (count < csv->count) {
        csv->substrings[count].start = c;
        csv->substrings[count].len = 0;
        while (*c != ',' && *c != '\0') {
            ++c;
            ++(csv->substrings[count].len);
        }
        ++count;
        ++c;
    }
    return count;
}

gsize csv_strlcpy(gchar *dest, const struct csv_substring *src, gsize dest_size) {
    gsize count = MIN(src->len, dest_size - 1);
    strncpy(dest, src->start, count);
    dest[count] = '\0';
    return count;
}

gint csv_getint(const struct csv_substring *src) {
    return atoi(src->start);
}

void csv_free(csv_t *csv) {
    g_free(csv->substrings);
    g_free(csv);
};

// ---------------------------------------------------------

#define STRUCTURES_CSV_COUNT 6
#define TERRAIN_CSV_COUNT 2
#define OVERLAY_CSV_COUNT 1
#define UNITS_CSV_COUNT 7

void parse_structures(map_t *dest, ini_t *ini);
void parse_terrain(map_t *dest, ini_t *ini);
void parse_overlay(map_t *dest, ini_t *ini);
void parse_units(map_t *dest, ini_t *ini);

gint compare_entity(gconstpointer a, gconstpointer b, gpointer user_data) {
    if (((entity_t *)a)->location < ((entity_t *)b)->location) return -1;
    else return ((entity_t *)a)->location - ((entity_t *)b)->location;
}

gint compare_terrain(gconstpointer a, gconstpointer b, gpointer user_data) {
    if (((terrain_t *)a)->location < ((terrain_t *)b)->location) return -1;
    else return ((terrain_t *)a)->location - ((terrain_t *)b)->location;
}

void map_parse(map_t *dest, ini_t *ini, const gchar bin[]) {
    dest->map.height = atoi(ini_get(ini, "MAP", "Height"));
    dest->map.width = atoi(ini_get(ini, "MAP", "Width"));
    dest->map.x = atoi(ini_get(ini, "MAP", "X"));
    dest->map.y = atoi(ini_get(ini, "MAP", "Y"));
    strncpy(dest->map.theater, ini_get(ini, "MAP", "Theater"), sizeof(dest->map.theater)-1);
    dest->bin = bin;

    dest->entities = g_sequence_new(g_free);
    dest->terrain = g_sequence_new(g_free);

    parse_structures(dest, ini);
    parse_overlay(dest, ini);
    parse_units(dest, ini);

    g_sequence_sort(dest->entities, compare_entity, NULL);

    parse_terrain(dest, ini);
    g_sequence_sort(dest->terrain, compare_terrain, NULL);
}

void parse_structures(map_t *dest, ini_t *ini) {
    csv_t csv;
    ini_iter_t *i = ini_iterate_section(ini, "STRUCTURES");
    if (i) {
        while (ini_iter_next(i)) {
            csv.data = ini_iter_get_value(i);
            if (csv_parse(&csv) != STRUCTURES_CSV_COUNT) {
                fprintf(stderr, "Unexpected data in STRUCTURES section\n");
                continue;
            }
            entity_t *e = g_new(entity_t, 1);
            e->kind = ENTITY_STRUCTURE;
            csv_strlcpy(e->data.structure.owner, &csv.substrings[0], sizeof(e->data.structure.owner));
            csv_strlcpy(e->type, &csv.substrings[1], sizeof(e->type));
            e->data.structure.health = csv_getint(&csv.substrings[2]);
            e->location = csv_getint(&csv.substrings[3]);
            e->data.structure.rotation = csv_getint(&csv.substrings[4]);
            g_sequence_append(dest->entities, e);
        }
        ini_iter_free(i);
    }
}

void parse_terrain(map_t *dest, ini_t *ini) {
    csv_t csv;
    ini_iter_t *i = ini_iterate_section(ini, "TERRAIN");
    if (i) {
        while (ini_iter_next(i)) {
            csv.data = ini_iter_get_value(i);
            if (csv_parse(&csv) != TERRAIN_CSV_COUNT) {
                fprintf(stderr, "Unexpected data in TERRAIN section\n");
                continue;
            }
            terrain_t *t = g_new(terrain_t, 1);
            t->location = atoi(ini_iter_get_key(i));
            csv_strlcpy(t->type, &csv.substrings[0], sizeof(t->type));
            g_sequence_append(dest->terrain, t);
        }
        ini_iter_free(i);
    }
}

void update_overlay_neighbours(GTree *overlaymap, entity_t *ov) {
    // TODO account for tiberium
    entity_t *neighbour;
    gint k;
    ov->data.overlay.neighbours = 0;
    // Above
    k = ov->location - MAP_WIDTH;
    if (ov->location > (MAP_WIDTH - 1) && (neighbour = g_tree_lookup(overlaymap, &k))) {
        if (strncmp(neighbour->type, ov->type, 4) == 0) {
            neighbour->data.overlay.neighbours |= NEIGHBOUR_BOTTOM;
            ov->data.overlay.neighbours |= NEIGHBOUR_TOP;
        }
    }
    // Left
    k = ov->location - 1;
    if ((ov->location % MAP_WIDTH) != 0 && (neighbour = g_tree_lookup(overlaymap, &k))) {
        if (strncmp(neighbour->type, ov->type, 4) == 0) {
            neighbour->data.overlay.neighbours |= NEIGHBOUR_RIGHT;
            ov->data.overlay.neighbours |= NEIGHBOUR_LEFT;
        }
    }
    // Right
    k = ov->location + 1;
    if (ov->location % MAP_WIDTH != (MAP_WIDTH - 1) && (neighbour = g_tree_lookup(overlaymap, &k))) {
        if (strncmp(neighbour->type, ov->type, 4) == 0) {
            neighbour->data.overlay.neighbours |= NEIGHBOUR_LEFT;
            ov->data.overlay.neighbours |= NEIGHBOUR_RIGHT;
        }
    }
    // Below
    k = ov->location + MAP_WIDTH;
    if (ov->location < ((MAP_HEIGHT - 1) * MAP_WIDTH) && (neighbour = g_tree_lookup(overlaymap, &k))) {
        if (strncmp(neighbour->type, ov->type, 4) == 0) {
            neighbour->data.overlay.neighbours |= NEIGHBOUR_TOP;
            ov->data.overlay.neighbours |= NEIGHBOUR_BOTTOM;
        }
    }
};

gint overlaymap_key_compare(gconstpointer a, gconstpointer b, gpointer user_data) {
    if (*(const gushort *)a < *(const gushort *)b) return -1;
    return *(const gushort *)a - *(const gushort *)b;
}

void parse_overlay(map_t *dest, ini_t *ini) {
    csv_t csv;
    GTree *overlaymap = g_tree_new_full(overlaymap_key_compare, NULL, g_free, NULL);
    ini_iter_t *i = ini_iterate_section(ini, "OVERLAY");
    if (i) {
        while (ini_iter_next(i)) {
            csv.data = ini_iter_get_value(i);
            if (csv_parse(&csv) != OVERLAY_CSV_COUNT) {
                fprintf(stderr, "Unexpected data in OVERLAY section\n");
                continue;
            }
            gushort *location = g_new(gushort, 1);
            entity_t *e = g_new(entity_t, 1);
            e->kind = ENTITY_OVERLAY;
            *location = atoi(ini_iter_get_key(i));
            e->location = *location;
            csv_strlcpy(e->type, &csv.substrings[0], sizeof(e->type));
            update_overlay_neighbours(overlaymap, e);
            g_sequence_append(dest->entities, e);
            g_tree_insert(overlaymap, location, e);
        }
        ini_iter_free(i);
    }
    g_tree_destroy(overlaymap);
}

void parse_units(map_t *dest, ini_t *ini) {
    csv_t csv;
    ini_iter_t *i = ini_iterate_section(ini, "UNITS");
    if (i) {
        while (ini_iter_next(i)) {
            csv.data = ini_iter_get_value(i);
            if (csv_parse(&csv) != UNITS_CSV_COUNT) {
                fprintf(stderr, "Unexpected data in UNITS section\n");
                continue;
            }
            entity_t *e = g_new(entity_t, 1);
            e->kind = ENTITY_UNIT;
            csv_strlcpy(e->data.unit.owner, &csv.substrings[0], sizeof(e->data.unit.owner));
            csv_strlcpy(e->type, &csv.substrings[1], sizeof(e->type));
            e->data.unit.health = csv_getint(&csv.substrings[2]);
            e->location = csv_getint(&csv.substrings[3]);
            e->data.unit.rotation = csv_getint(&csv.substrings[4]);
            //csv_strlcpy(unit->trigger, &csv.substrings[5], sizeof(unit->trigger));
            g_sequence_append(dest->entities, e);
        }
        ini_iter_free(i);
    }
}
