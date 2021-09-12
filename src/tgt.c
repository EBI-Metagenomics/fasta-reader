#include "far/tgt.h"
#include "aux.h"
#include "error.h"
#include "far/far.h"
#include "far/state.h"
#include "far/tok.h"
#include "fsm.h"
#include "tgt.h"
#include "tok.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

void far_tgt_write(char const *id, char const *desc, char const *seq,
                   unsigned ncols, FILE *restrict fd)
{
    fprintf(fd, ">%s", id);
    if (desc[0]) fprintf(fd, " %s", desc);

    for (char const *c = seq; *c; ++c)
    {
        if (((c - seq) % ncols) == 0)
        {
            fputc('\n', fd);
        }
        fputc(*c, fd);
    }
    fputc('\n', fd);
}

void far_tgt_init(struct far_tgt *tgt, struct far *far)
{
    tgt_init(tgt, far->error);
}

void tgt_init(struct far_tgt *tgt, char *error)
{
    tgt->id[0] = '\0';
    tgt->desc[0] = '\0';
    tgt->seq[0] = '\0';
    tgt->error = error;
}

enum far_rc tgt_next(struct far_tgt *tgt, FILE *restrict fd,
                     struct far_aux *aux, enum far_state *state,
                     struct far_tok *tok)
{
    if (*state == FAR_FSM_END) return FAR_ENDFILE;

    if (*state != FAR_FSM_BEGIN && *state != FAR_FSM_PAUSE)
        return error_runtime(tgt->error, "unexpected %s call", __func__);

    tgt_init(tgt, tok->error);
    if (*state == FAR_FSM_PAUSE) strcpy(tgt->id, aux->id);

    enum far_state initial_state = *state;
    do
    {
        enum far_rc rc = FAR_SUCCESS;
        if ((rc = tok_next(tok, fd))) return rc;

        if ((*state = fsm_next(*state, tok, aux, tgt)) == FAR_FSM_ERROR)
            return FAR_PARSEERROR;

    } while (*state != FAR_FSM_PAUSE && *state != FAR_FSM_END);

    if (*state == FAR_FSM_END && initial_state == FAR_FSM_BEGIN)
        return FAR_ENDFILE;

    return FAR_SUCCESS;
}
