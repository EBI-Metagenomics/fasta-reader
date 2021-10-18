#include "far/far.h"
#include "fsm.h"
#include "tgt.h"
#include "tok.h"

void fasta_init(struct fasta *fa, FILE *restrict fd, enum fasta_mode mode)
{
    fa->fd = fd;
    fa->mode = mode;
    fsm_init(&fa->state);
    fa->error[0] = '\0';
    tok_init(&fa->tok, fa->error);
}

enum fasta_rc fasta_next_target(struct fasta *fa, struct fasta_target *tgt)
{
    return target_next(tgt, fa->fd, &fa->aux, &fa->state, &fa->tok);
}

void fasta_clear_error(struct fasta *fa) { fa->error[0] = '\0'; }
