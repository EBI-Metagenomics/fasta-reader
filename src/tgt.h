#ifndef TGT_H
#define TGT_H

#include "far/rc.h"
#include <stdio.h>

enum state;
struct fasta_aux;
struct fasta_target;
struct fasta_tok;

void target_init(struct fasta_target *tgt, char *error);

enum fasta_rc target_next(struct fasta_target *tgt, FILE *restrict fd,
                          struct fasta_aux *aux, enum state *state,
                          struct fasta_tok *tok);

#endif
