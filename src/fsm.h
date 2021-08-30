#ifndef FSM_H
#define FSM_H

#include "far/state.h"

struct far_aux;
struct far_tgt;
struct far_tok;

static inline void fsm_init(enum far_state *state) { *state = FAR_FSM_BEGIN; }

enum far_state fsm_next(enum far_state state, struct far_tok *tok,
                        struct far_aux *aux, struct far_tgt *prof);

char const *fsm_name(enum far_state state);

#endif
