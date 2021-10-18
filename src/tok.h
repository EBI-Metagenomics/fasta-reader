#ifndef TOK_H
#define TOK_H

#include "far/rc.h"
#include <stdio.h>

struct fasta_tok;

void tok_init(struct fasta_tok *tok, char *error);
enum fasta_rc tok_next(struct fasta_tok *tok, FILE *restrict fd);

#endif
