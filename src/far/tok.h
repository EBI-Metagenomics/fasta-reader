#ifndef FAR_TOK_H
#define FAR_TOK_H

#include <stdbool.h>

enum far_tok_id
{
    FAR_TOK_NL,
    FAR_TOK_ID,
    FAR_TOK_WORD,
    FAR_TOK_EOF,
};

#define FAR_TOK_LINE_MAX 256

struct far_tok
{
    enum far_tok_id id;
    char const *value;
    struct
    {
        char data[FAR_TOK_LINE_MAX];
        unsigned number;
        bool consumed;
        char *ctx;
    } line;
    char *error;
};

#endif
