#include "ini.h"

struct ini {
    GHashTable *sections;
};

void destroy_str(gpointer data) {
    g_free(data);
}

void destroy_section(gpointer data) {
    g_hash_table_unref((GHashTable *)data);
}

ini_t *ini_new() {
    ini_t *ini = g_new(ini_t, 1);
    ini->sections = g_hash_table_new_full(g_str_hash, g_str_equal, destroy_str, destroy_section);
    return ini; // TODO implement ini_new
}

void ini_free(ini_t *ini) {
    if (ini != NULL) {
        if (ini->sections != NULL) {
            g_hash_table_unref(ini->sections);
        }
        g_free(ini);
    }
}

void ini_add_section(ini_t *ini, gchar *name) {
    GHashTable *section = g_hash_table_new_full(g_str_hash, g_str_equal, destroy_str, destroy_str);
    g_hash_table_insert(ini->sections, name, section);
}

void ini_add_item(ini_t *ini, gchar *section, gchar *key, gchar *value) {
    if (!g_hash_table_contains(ini->sections, section)) {
        ini_add_section(ini, section); // XXX: This may cause some weird memory issues (who owns section?)
    }
    GHashTable *sect = (GHashTable *)g_hash_table_lookup(ini->sections, section);
    g_hash_table_insert(sect, key, value);
}

gboolean ini_has_section(ini_t *ini, const gchar *name) {
    return g_hash_table_contains(ini->sections, name);
}

gboolean ini_section_has_key(ini_t *ini, const gchar *section, const gchar *key) {
    GHashTable *sect = NULL;
    gboolean section_exists = g_hash_table_contains(ini->sections, section);
    if (section_exists) {
        sect = g_hash_table_lookup(ini->sections, section);
        return g_hash_table_contains(sect, key);
    } else {
        return FALSE;
    }
}

const gchar* ini_get(ini_t *ini, const gchar* section, const gchar *key) {
    if (ini_section_has_key(ini, section, key)) {
        GHashTable *sect = g_hash_table_lookup(ini->sections, section);
        return g_hash_table_lookup(sect, key);
    } else {
        return NULL;
    }
}

// ---------------------------------------------------------

struct ini_iter {
    GHashTableIter g_iter;
    gpointer key;
    gpointer value;
};

ini_iter_t *ini_iterate_section(ini_t *ini, const gchar *section) {
    GHashTable *sect = g_hash_table_lookup(ini->sections, section);
    if (!sect) {
        return NULL;
    }
    ini_iter_t *iter = g_new0(ini_iter_t, 1);
    g_hash_table_iter_init(&iter->g_iter, sect);
    return iter;
}

gboolean ini_iter_next(ini_iter_t *iter) {
    return g_hash_table_iter_next(&iter->g_iter, &iter->key, &iter->value);
}

const gchar *ini_iter_get_key(ini_iter_t *iter) {
    return (const gchar *)iter->key;
}

const gchar *ini_iter_get_value(ini_iter_t *iter) {
    return (const gchar *)iter->value;
}

void ini_iter_free(ini_iter_t *iter) {
    g_free(iter);
}

