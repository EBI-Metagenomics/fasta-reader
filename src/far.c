#include "far/far.h"
#include "fsm.h"
#include "tgt.h"
#include "tok.h"

void far_init(struct far *far, FILE *restrict fd)
{
    far->fd = fd;
    fsm_init(&far->state);
    far->error[0] = '\0';
    tok_init(&far->tok, far->error);
}

enum far_rc far_next_tgt(struct far *far, struct far_tgt *tgt)
{
    return tgt_next(tgt, far->fd, &far->aux, &far->state, &far->tok);
}

void far_clear_error(struct far *far) { far->error[0] = '\0'; }
