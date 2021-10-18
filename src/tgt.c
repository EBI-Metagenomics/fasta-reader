#include "far/tgt.h"
#include "aux.h"
#include "error.h"
#include "far/far.h"
#include "far/tok.h"
#include "fsm.h"
#include "state.h"
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

static void buf_init(struct fasta_target *tgt)
{
    tgt->buf.id[0] = '\0';
    tgt->buf.desc[0] = '\0';
    tgt->buf.seq[0] = '\0';
}

void fasta_target_init(struct fasta_target *tgt, struct fasta *fa)
{
    buf_init(tgt);
    tgt->id = tgt->buf.id;
    tgt->desc = tgt->buf.desc;
    tgt->seq = tgt->buf.seq;
    tgt->error = fa->error;
}

enum fasta_rc target_next(struct fasta_target *tgt, FILE *restrict fd,
                          struct fasta_aux *aux, enum state *state,
                          struct fasta_tok *tok)
{
    if (*state == STATE_END) return FAR_ENDFILE;

    if (*state != STATE_BEGIN && *state != STATE_PAUSE)
        return error_runtime(tgt->error, "unexpected %s call", __func__);

    buf_init(tgt);
    if (*state == STATE_PAUSE) strcpy(tgt->id, aux->id);

    enum state initial_state = *state;
    do
    {
        enum fasta_rc rc = FAR_SUCCESS;
        if ((rc = tok_next(tok, fd))) return rc;

        if ((*state = fsm_next(*state, tok, aux, tgt)) == STATE_ERROR)
            return FAR_PARSEERROR;

    } while (*state != STATE_PAUSE && *state != STATE_END);

    if (*state == STATE_END && initial_state == STATE_BEGIN) return FAR_ENDFILE;

    return FAR_SUCCESS;
}
