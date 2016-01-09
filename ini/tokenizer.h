#ifndef MHENLO_INI_TOKENIZER_H_INCLUDED
#define MHENLO_INI_TOKENIZER_H_INCLUDED 

#include <glib.h>

enum IniTokenType {
    INI_TOKEN_WHITESPACE = ' ',
    INI_TOKEN_BRACKET_LEFT = '[',
    INI_TOKEN_BRACKET_RIGHT = ']',
    INI_TOKEN_EQUALS = '=',
    INI_TOKEN_SEMICOLON = ';',
    INI_TOKEN_TEXT = 0x02, // ASCII: Start of text
    INI_TOKEN_EOL = '\n',
    INI_TOKEN_EOF = 0x04 // ASCII: End of transmission
};

typedef struct ini_token {
    enum IniTokenType type;
    struct {
        gchar *value;
        gsize len;
    } text;
} ini_token_t;

struct ini_tokenizer;
typedef struct ini_tokenizer ini_tokenizer_t;

ini_tokenizer_t *ini_tokenizer_new(const gchar *buf, gsize len);
void ini_tokenizer_free(ini_tokenizer_t *tokenizer);
ini_token_t *ini_tokenizer_next(ini_tokenizer_t *tokenizer);
ini_token_t *ini_tokenizer_peek(ini_tokenizer_t *tokenizer);

#endif /* MHENLO_INI_TOKENIZER_H_INCLUDED */
