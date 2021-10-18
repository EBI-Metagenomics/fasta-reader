#ifndef FSM_H
#define FSM_H

#include "state.h"

struct fasta_aux;
struct fasta_target;
struct fasta_tok;

static inline void fsm_init(enum state *state) { *state = STATE_BEGIN; }

enum state fsm_next(enum state state, struct fasta_tok *tok,
                    struct fasta_aux *aux, struct fasta_target *tgt);

char const *fsm_name(enum state state);

#endif
