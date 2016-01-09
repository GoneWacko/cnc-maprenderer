#ifndef MHENLO_INI_INI_H_INCLUDED
#define MHENLO_INI_INI_H_INCLUDED 
#include <glib.h>

struct ini;
typedef struct ini ini_t;

struct ini_iter;
typedef struct ini_iter ini_iter_t;

ini_t *ini_new();
void ini_free(ini_t *ini);

void ini_add_section(ini_t *ini, gchar *name);
void ini_add_item(ini_t *ini, gchar *section, gchar *key, gchar *value);

gboolean ini_has_section(ini_t *ini, const gchar *name);
gboolean ini_section_has_key(ini_t *ini, const gchar *section, const gchar *key);
const gchar* ini_get(ini_t *ini, const gchar* section, const gchar *key);

ini_iter_t *ini_iterate_section(ini_t *ini, const gchar *section);
gboolean ini_iter_next(ini_iter_t *iter);
const gchar *ini_iter_get_key(ini_iter_t *iter);
const gchar *ini_iter_get_value(ini_iter_t *iter);
void ini_iter_free(ini_iter_t *iter);

#endif /* MHENLO_INI_INI_H_INCLUDED */
