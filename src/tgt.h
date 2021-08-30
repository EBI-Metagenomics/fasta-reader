#ifndef TGT_H
#define TGT_H

#include "far/rc.h"
#include <stdio.h>

enum far_state;
struct far_aux;
struct far_tgt;
struct far_tok;

void tgt_init(struct far_tgt *tgt, char *error);

enum far_rc tgt_next(struct far_tgt *tgt, FILE *restrict fd,
                     struct far_aux *aux, enum far_state *state,
                     struct far_tok *tok);

#endif
