#include "far/prof.h"
#include "aux.h"
#include "bug.h"
#include "error.h"
#include "far/far.h"
#include "far/tok.h"
#include "fsm.h"
#include "node.h"
#include "prof.h"
#include "tok.h"
#include <limits.h>
#include <stdlib.h>

void far_prof_dump(struct far_prof const *prof, FILE *restrict fd)
{
    fprintf(fd, "HEADER: %s\n", prof->header);
    fprintf(fd, "  Name: %s\n", prof->meta.name);
    fprintf(fd, "   Acc: %s\n", prof->meta.acc);
    fprintf(fd, "  Desc: %s\n", prof->meta.desc);
    fprintf(fd, "  Leng: %s\n", prof->meta.leng);
    fprintf(fd, "  Alph: %s\n", prof->meta.alph);
    fprintf(fd, "  Name: %s\n", prof->meta.name);
    fprintf(fd, "  ");
    for (unsigned i = 0; i < prof->symbols_size; ++i)
    {
        fprintf(fd, "       %c", prof->symbols[i]);
    }
    fprintf(fd, "\n");
}

void far_prof_init(struct far_prof *prof, struct far *far)
{
    prof_init(prof, far->error);
}

void prof_init(struct far_prof *prof, char *error)
{
    prof->header[0] = '\0';
    prof->meta.name[0] = '\0';
    prof->meta.acc[0] = '\0';
    prof->meta.desc[0] = '\0';
    prof->meta.leng[0] = '\0';
    prof->meta.alph[0] = '\0';
    prof->symbols_size = 0;
    prof->symbols[0] = '\0';
    prof->error = error;
    node_init(&prof->node);
}

enum far_rc prof_next_node(struct far_prof *prof, FILE *restrict fd,
                           struct far_aux *aux, enum far_state *state,
                           struct far_tok *tok)
{
    if (*state != FAR_FSM_PAUSE)
        return error_runtime(prof->error, "unexpected %s call", __func__);

    aux_init(aux);
    do
    {
        enum far_rc rc = FAR_SUCCESS;
        if ((rc = tok_next(tok, fd))) return rc;

        *state = fsm_next(*state, tok, aux, prof);
        if (*state == FAR_FSM_ERROR) return FAR_PARSEERROR;
        if (*state == FAR_FSM_BEGIN)
        {
            if (far_prof_length(prof) != prof->node.idx)
            {
                return error_parse(tok, "profile length mismatch");
            }
            return FAR_ENDNODE;
        }

    } while (*state != FAR_FSM_PAUSE);

    return FAR_SUCCESS;
}

enum far_rc prof_next_prof(struct far_prof *prof, FILE *restrict fd,
                           struct far_aux *aux, enum far_state *state,
                           struct far_tok *tok)
{
    if (*state != FAR_FSM_BEGIN)
        return error_runtime(prof->error, "unexpected %s call", __func__);

    prof_init(prof, tok->error);
    aux_init(aux);
    do
    {
        enum far_rc rc = FAR_SUCCESS;
        if ((rc = tok_next(tok, fd))) return rc;

        if ((*state = fsm_next(*state, tok, aux, prof)) == FAR_FSM_ERROR)
            return FAR_PARSEERROR;

    } while (*state != FAR_FSM_PAUSE && *state != FAR_FSM_END);

    if (*state == FAR_FSM_END) return FAR_ENDFILE;

    return FAR_SUCCESS;
}

unsigned far_prof_length(struct far_prof const *prof)
{
    long v = strtol(prof->meta.leng, NULL, 10);
    if (v == LONG_MAX) return UINT_MAX;
    if (v == LONG_MIN) return 0;
    return (unsigned)v;
}
