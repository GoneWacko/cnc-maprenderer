#include "tokenizer.h"
#include <string.h>

struct ini_tokenizer {
    const gchar *buf;
    gsize len;

    gsize pos;

    gboolean peeked;
    ini_token_t *last_token;
};

ini_tokenizer_t *ini_tokenizer_new(const gchar *buf, gsize len) {
    if (buf == NULL) {
        return NULL;
    }
    ini_tokenizer_t *tokenizer = g_new0(ini_tokenizer_t, 1);
    if (tokenizer != NULL) {
        tokenizer->buf = buf;
        tokenizer->len = len;
    }
    return tokenizer;
}

void ini_tokenizer_free(ini_tokenizer_t *tokenizer) {
    if (tokenizer != NULL) {
        if (tokenizer->last_token != NULL) {
            g_free(tokenizer->last_token);
        }
        g_free(tokenizer);
    }
}

ini_token_t *ini_tokenizer_next(ini_tokenizer_t *tokenizer) {
    ini_token_t *token = ini_tokenizer_peek(tokenizer);
    if (token == NULL || token->type == INI_TOKEN_EOF) {
        return token;
    }

    tokenizer->pos += token->text.len;

    tokenizer->peeked = FALSE;
    return token;
}

enum IniTokenType ini_get_token_type(const gchar c) {
    switch (c) {
        case ' ':
        case '\t':
            return INI_TOKEN_WHITESPACE;
        case '[':
            return INI_TOKEN_BRACKET_LEFT;
        case ']':
            return INI_TOKEN_BRACKET_RIGHT;
        case '=':
            return INI_TOKEN_EQUALS;
        case ';':
            return INI_TOKEN_SEMICOLON;
        case '\r':
        case '\n':
            return INI_TOKEN_EOL;
        default:
            return INI_TOKEN_TEXT;
    }
}

ini_token_t *ini_tokenizer_peek(ini_tokenizer_t *tokenizer) {
    if (!tokenizer->peeked) {
        ini_token_t *token = g_new0(ini_token_t, 1);
        if (token == NULL) {
            return NULL;
        }

        if (tokenizer->pos >= tokenizer->len) {
            token->type = INI_TOKEN_EOF;
            token->text.len = 0;
        } else {
            token->type = ini_get_token_type(tokenizer->buf[tokenizer->pos]);
            if (token->type == INI_TOKEN_TEXT) {
                gsize p = tokenizer->pos;
                while (++p < tokenizer->len) {
                    if (ini_get_token_type(tokenizer->buf[p]) != INI_TOKEN_TEXT) {
                        break;
                    }
                }
                token->text.len = p - tokenizer->pos;
                token->text.value = g_malloc(token->text.len);    // XXX: Make this zero-terminated?
                memcpy(token->text.value, tokenizer->buf + tokenizer->pos, token->text.len);
            } else {
                // XXX: Ugly hack for \r\n newlines
                if (tokenizer->buf[tokenizer->pos] == '\r' && tokenizer->pos + 1 < tokenizer->len && tokenizer->buf[tokenizer->pos+1] == '\n') {
                    token->text.len = 2;
                } else {
                    token->text.len = 1;
                }
            }
        }

        tokenizer->last_token = token;
    }
    tokenizer->peeked = TRUE;
    return tokenizer->last_token;
}

