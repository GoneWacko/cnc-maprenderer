#include "parser.h"
#include "tokenizer.h"
#include "ini.h"

#define INI_INITIAL_VALUE_LENGTH 8

struct ini_parser {
    gchar *curr_section;
    gchar *curr_key;

    ini_token_t *curr_token;
    ini_tokenizer_t *tokenizer;

    ini_t *ini;
};

void skip_whitespace(ini_parser_t *parser);
void skip_comment(ini_parser_t *parser);
void parse_section_header(ini_parser_t *parser);
void parse_key_value_pair(ini_parser_t *parser);
void parse_value(ini_parser_t *parser);

ini_token_t *next_token(ini_parser_t *parser) {
    return parser->curr_token = ini_tokenizer_next(parser->tokenizer);
}

ini_t *ini_parse(const gchar *buf, gsize len) {
    ini_parser_t *parser;
    if (buf == NULL || len == 0) {
        return NULL;
    }

    parser = g_new0(ini_parser_t, 1);
    parser->tokenizer = ini_tokenizer_new(buf, len);
    parser->ini = ini_new();

    while (next_token(parser)->type != INI_TOKEN_EOF) {
        skip_whitespace(parser);
        switch (parser->curr_token->type) {
            case INI_TOKEN_EOL:
            case INI_TOKEN_EOF:
                continue;
            case INI_TOKEN_BRACKET_LEFT:
                parse_section_header(parser);
                break;
            case INI_TOKEN_TEXT:
                parse_key_value_pair(parser);
                break;
            case INI_TOKEN_SEMICOLON:
                skip_comment(parser);
                break;
            default:
                // XXX: Error. Log error and continue or shut down?
                break;
        }
    }

    return parser->ini;
};

void skip_whitespace(ini_parser_t *parser) {
    while (parser->curr_token->type == INI_TOKEN_WHITESPACE) {
        next_token(parser);
    }
}

void skip_comment(ini_parser_t *parser) {
    while (parser->curr_token->type != INI_TOKEN_EOL && parser->curr_token->type != INI_TOKEN_EOF) {
        next_token(parser);
    }
}

void parse_section_header(ini_parser_t *parser) {
    // XXX: Assert that current token is left bracket?
    next_token(parser);
    gchar *name;
    gsize len = 0;
    switch (parser->curr_token->type) {
        case INI_TOKEN_BRACKET_RIGHT:
            name = g_strndup("", 0);
            break;
        case INI_TOKEN_TEXT:
            len = parser->curr_token->text.len;
            name = g_strndup(parser->curr_token->text.value, len);
            break;
        default:
            // Raise error
            break;
    }
    if (next_token(parser)->type == INI_TOKEN_BRACKET_RIGHT) {
        parser->curr_section = name;
        ini_add_section(parser->ini, name);
    } else {
        g_free(name);
        // TODO: Raise error
    }
}

void parse_key_value_pair(ini_parser_t *parser) {
    gchar *key;

    key = g_strndup(parser->curr_token->text.value, parser->curr_token->text.len);
    next_token(parser);
    skip_whitespace(parser);
    if (parser->curr_token->type == INI_TOKEN_EQUALS) {
        next_token(parser);
        skip_whitespace(parser);

        parser->curr_key = key;
        parse_value(parser);
    }
}

void parse_value(ini_parser_t *parser) {
    GString *value = g_string_sized_new(INI_INITIAL_VALUE_LENGTH);
    while (parser->curr_token->type != INI_TOKEN_EOL && parser->curr_token->type != INI_TOKEN_EOF && parser->curr_token->type != INI_TOKEN_SEMICOLON) {
        // TODO: Tabs will get replaced with a single space as it stands now, fix this if it's a problem
        if (parser->curr_token->type == INI_TOKEN_TEXT) {
            g_string_append_len(value, parser->curr_token->text.value, parser->curr_token->text.len);
        } else {
            g_string_append_c(value, (gchar)parser->curr_token->type);
        }
        next_token(parser);
    }

    ini_add_item(parser->ini, parser->curr_section, parser->curr_key, value->str);
    g_string_free(value, FALSE);
}


