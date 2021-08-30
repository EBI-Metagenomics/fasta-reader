#ifndef TOK_H
#define TOK_H

#include "far/rc.h"
#include <stdio.h>

struct far_tok;

void tok_init(struct far_tok *tok, char *error);
enum far_rc tok_next(struct far_tok *tok, FILE *restrict fd);

#endif
