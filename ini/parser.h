#ifndef MHENLO_INI_PARSER_H_INCLUDED
#define MHENLO_INI_PARSER_H_INCLUDED 

#include <glib.h>

struct ini;
typedef struct ini ini_t;

struct ini_parser;
typedef struct ini_parser ini_parser_t;

ini_t *ini_parse(const gchar *buf, gsize len);

#endif /* MHENLO_INI_PARSER_H_INCLUDED */
