#ifndef PROF_H
#define PROF_H

#include "far/rc.h"
#include <stdio.h>

enum far_state;
struct far_aux;
struct far_prof;
struct far_tok;

void prof_init(struct far_prof *prof, char *error);

enum far_rc prof_next_node(struct far_prof *prof, FILE *restrict fd,
                           struct far_aux *aux, enum far_state *state,
                           struct far_tok *tok);

enum far_rc prof_next_prof(struct far_prof *prof, FILE *restrict fd,
                           struct far_aux *aux, enum far_state *state,
                           struct far_tok *tok);

#endif
