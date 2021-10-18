#include "fsm.h"
#include "aux.h"
#include "error.h"
#include "far/aux.h"
#include "far/tgt.h"
#include "far/tok.h"
#include "tok.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define FAR_MATCH_EXCESS_SIZE 5

struct args
{
    struct fasta_tok *tok;
    enum state state;
    struct fasta_aux *aux;
    struct fasta_target *tgt;
};

struct trans
{
    enum state const next;
    enum fasta_rc (*action)(struct args *a);
};

static enum fasta_rc nop(struct args *a) { return FAR_SUCCESS; }

static enum fasta_rc unexpect_eof(struct args *a)
{
    return error_parse(a->tok, "unexpected end-of-file");
}

static enum fasta_rc unexpect_tok(struct args *a)
{
    return error_parse(a->tok, "unexpected token");
}

static enum fasta_rc unexpect_id(struct args *a)
{
    return error_parse(a->tok, "unexpected id");
}

static enum fasta_rc unexpect_nl(struct args *a)
{
    return error_parse(a->tok, "unexpected newline");
}

static enum fasta_rc desc_begin(struct args *a);
static enum fasta_rc desc_cont(struct args *a);
static enum fasta_rc read_id(struct args *a);
static enum fasta_rc seq_begin(struct args *a);
static enum fasta_rc seq_cont(struct args *a);
static enum fasta_rc store_id(struct args *a);

static struct trans const transition[][4] = {
    [STATE_BEGIN] = {[FAR_TOK_NL] = {STATE_BEGIN, &nop},
                     [FAR_TOK_ID] = {STATE_DESC_BEGIN, &read_id},
                     [FAR_TOK_WORD] = {STATE_ERROR, &unexpect_tok},
                     [FAR_TOK_EOF] = {STATE_END, &nop}},
    [STATE_DESC_BEGIN] = {[FAR_TOK_NL] = {STATE_SEQ_BEGIN, &nop},
                          [FAR_TOK_ID] = {STATE_DESC_CONT, &desc_begin},
                          [FAR_TOK_WORD] = {STATE_DESC_CONT, &desc_begin},
                          [FAR_TOK_EOF] = {STATE_ERROR, &unexpect_eof}},
    [STATE_DESC_CONT] = {[FAR_TOK_NL] = {STATE_SEQ_BEGIN, &nop},
                         [FAR_TOK_ID] = {STATE_DESC_CONT, &desc_cont},
                         [FAR_TOK_WORD] = {STATE_DESC_CONT, &desc_cont},
                         [FAR_TOK_EOF] = {STATE_ERROR, &unexpect_eof}},
    [STATE_SEQ_BEGIN] = {[FAR_TOK_NL] = {STATE_ERROR, &unexpect_nl},
                         [FAR_TOK_ID] = {STATE_ERROR, &unexpect_id},
                         [FAR_TOK_WORD] = {STATE_SEQ_NL, &seq_begin},
                         [FAR_TOK_EOF] = {STATE_ERROR, &unexpect_eof}},
    [STATE_SEQ_NL] = {[FAR_TOK_NL] = {STATE_SEQ_CONT, &nop},
                      [FAR_TOK_ID] = {STATE_ERROR, &unexpect_id},
                      [FAR_TOK_WORD] = {STATE_ERROR, &unexpect_tok},
                      [FAR_TOK_EOF] = {STATE_END, &nop}},
    [STATE_SEQ_CONT] = {[FAR_TOK_NL] = {STATE_NL, &nop},
                        [FAR_TOK_ID] = {STATE_PAUSE, &store_id},
                        [FAR_TOK_WORD] = {STATE_SEQ_NL, &seq_cont},
                        [FAR_TOK_EOF] = {STATE_END, &nop}},
    [STATE_NL] = {[FAR_TOK_NL] = {STATE_NL, &nop},
                  [FAR_TOK_ID] = {STATE_PAUSE, &store_id},
                  [FAR_TOK_WORD] = {STATE_ERROR, &unexpect_tok},
                  [FAR_TOK_EOF] = {STATE_END, &nop}},
    [STATE_PAUSE] = {[FAR_TOK_NL] = {STATE_SEQ_BEGIN, &nop},
                     [FAR_TOK_ID] = {STATE_DESC_CONT, &desc_begin},
                     [FAR_TOK_WORD] = {STATE_DESC_CONT, &desc_begin},
                     [FAR_TOK_EOF] = {STATE_ERROR, &unexpect_eof}},
    [STATE_END] = {[FAR_TOK_NL] = {STATE_ERROR, &unexpect_nl},
                   [FAR_TOK_ID] = {STATE_ERROR, &unexpect_id},
                   [FAR_TOK_WORD] = {STATE_ERROR, &unexpect_tok},
                   [FAR_TOK_EOF] = {STATE_ERROR, &unexpect_eof}},
    [STATE_ERROR] = {[FAR_TOK_NL] = {STATE_ERROR, &nop},
                     [FAR_TOK_ID] = {STATE_ERROR, &nop},
                     [FAR_TOK_WORD] = {STATE_ERROR, &nop},
                     [FAR_TOK_EOF] = {STATE_ERROR, &nop}},
};

static char state_name[][14] = {
    [STATE_BEGIN] = "BEGIN",
    [STATE_DESC_BEGIN] = "DESC_BEGIN",
    [STATE_DESC_CONT] = "DESC_CONT",
    [STATE_SEQ_BEGIN] = "SEQ_BEGIN",
    [STATE_SEQ_NL] = "SEQ_NL",
    [STATE_SEQ_CONT] = "SEQ_CONT",
    [STATE_NL] = "NL",
    [STATE_PAUSE] = "PAUSE",
    [STATE_END] = "END",
    [STATE_ERROR] = "ERROR",
};

enum state fsm_next(enum state state, struct fasta_tok *tok,
                    struct fasta_aux *aux, struct fasta_target *tgt)
{
    unsigned row = (unsigned)state;
    unsigned col = (unsigned)tok->id;
    struct trans const *const t = &transition[row][col];
    struct args args = {tok, state, aux, tgt};
    if (t->action(&args)) return STATE_ERROR;
    return t->next;
}

char const *fsm_name(enum state state) { return state_name[state]; }

static enum fasta_rc read_desc(struct args *a)
{
    a->aux->pos = memccpy(a->aux->pos - 1, a->tok->value, '\0',
                          (unsigned long)(a->aux->end - a->aux->pos));
    if (!a->aux->pos) return error_parse(a->tok, "too long description");
    return FAR_SUCCESS;
}

static enum fasta_rc desc_begin(struct args *a)
{
    assert(a->tok->id == FAR_TOK_WORD || a->tok->id == FAR_TOK_ID);
    a->aux->begin = a->tgt->desc;
    a->aux->pos = a->aux->begin + 1;
    a->aux->end = a->aux->begin + FAR_DESC_MAX;
    return read_desc(a);
}

static enum fasta_rc desc_cont(struct args *a)
{
    assert(a->tok->id == FAR_TOK_WORD || a->tok->id == FAR_TOK_ID);
    *(a->aux->pos - 1) = ' ';
    a->aux->pos++;
    return read_desc(a);
}

static enum fasta_rc read_id(struct args *a)
{
    assert(a->tok->id == FAR_TOK_ID);
    char const *ptr = memccpy(a->tgt->id, a->tok->value + 1, '\0', FAR_ID_MAX);
    if (!ptr) return error_parse(a->tok, "too long id");
    if (ptr - a->tgt->id == 1) return error_parse(a->tok, "empty id");
    return FAR_SUCCESS;
}

static enum fasta_rc seq_begin(struct args *a)
{
    assert(a->tok->id == FAR_TOK_WORD);
    a->aux->begin = a->tgt->seq;
    a->aux->pos = a->aux->begin + 1;
    a->aux->end = a->aux->begin + FAR_SEQ_MAX;
    return seq_cont(a);
}

static enum fasta_rc seq_cont(struct args *a)
{
    assert(a->tok->id == FAR_TOK_WORD);
    a->aux->pos = memccpy(a->aux->pos - 1, a->tok->value, '\0',
                          (unsigned long)(a->aux->end - a->aux->pos));
    if (!a->aux->pos) return error_parse(a->tok, "too long sequence");
    return FAR_SUCCESS;
}

static enum fasta_rc store_id(struct args *a)
{
    assert(a->tok->id == FAR_TOK_ID);
    char const *ptr = memccpy(a->aux->id, a->tok->value + 1, '\0', FAR_ID_MAX);
    if (!ptr) return error_parse(a->tok, "too long id");
    if (ptr - a->aux->id == 1) return error_parse(a->tok, "empty id");
    return FAR_SUCCESS;
}
