#include "fsm.h"
#include "aux.h"
#include "bug.h"
#include "error.h"
#include "far/aux.h"
#include "far/tgt.h"
#include "far/tok.h"
#include "tok.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define FAR_MATCH_EXCESS_SIZE 5

#define XSTR(s) STR(s)
#define STR(s) #s

#define DEC_ERROR "failed to parse decimal number"

struct args
{
    struct far_tok *tok;
    enum far_state state;
    struct far_aux *aux;
    struct far_tgt *tgt;
};

struct trans
{
    enum far_state const next;
    enum far_rc (*action)(struct args *a);
};

static enum far_rc nop(struct args *a) { return FAR_SUCCESS; }

static enum far_rc unexpect_eof(struct args *a)
{
    return error_parse(a->tok, "unexpected end-of-file");
}

static enum far_rc unexpect_tok(struct args *a)
{
    return error_parse(a->tok, "unexpected token");
}

static enum far_rc unexpect_id(struct args *a)
{
    return error_parse(a->tok, "unexpected id");
}

static enum far_rc unexpect_nl(struct args *a)
{
    return error_parse(a->tok, "unexpected newline");
}

static enum far_rc desc_begin(struct args *a);
static enum far_rc desc_cont(struct args *a);
static enum far_rc read_id(struct args *a);
static enum far_rc seq_begin(struct args *a);
static enum far_rc seq_cont(struct args *a);
static enum far_rc store_id(struct args *a);

static struct trans const transition[][4] = {
    [FAR_FSM_BEGIN] = {[FAR_TOK_NL] = {FAR_FSM_BEGIN, &nop},
                       [FAR_TOK_ID] = {FAR_FSM_DESC_BEGIN, &read_id},
                       [FAR_TOK_WORD] = {FAR_FSM_ERROR, &unexpect_tok},
                       [FAR_TOK_EOF] = {FAR_FSM_END, &nop}},
    [FAR_FSM_DESC_BEGIN] = {[FAR_TOK_NL] = {FAR_FSM_SEQ_BEGIN, &nop},
                            [FAR_TOK_ID] = {FAR_FSM_DESC_CONT, &desc_begin},
                            [FAR_TOK_WORD] = {FAR_FSM_DESC_CONT, &desc_begin},
                            [FAR_TOK_EOF] = {FAR_FSM_ERROR, &unexpect_eof}},
    [FAR_FSM_DESC_CONT] = {[FAR_TOK_NL] = {FAR_FSM_SEQ_BEGIN, &nop},
                           [FAR_TOK_ID] = {FAR_FSM_DESC_CONT, &desc_cont},
                           [FAR_TOK_WORD] = {FAR_FSM_DESC_CONT, &desc_cont},
                           [FAR_TOK_EOF] = {FAR_FSM_ERROR, &unexpect_eof}},
    [FAR_FSM_SEQ_BEGIN] = {[FAR_TOK_NL] = {FAR_FSM_ERROR, &unexpect_nl},
                           [FAR_TOK_ID] = {FAR_FSM_ERROR, &unexpect_id},
                           [FAR_TOK_WORD] = {FAR_FSM_SEQ_NL, &seq_begin},
                           [FAR_TOK_EOF] = {FAR_FSM_ERROR, &unexpect_eof}},
    [FAR_FSM_SEQ_NL] = {[FAR_TOK_NL] = {FAR_FSM_SEQ_CONT, &nop},
                        [FAR_TOK_ID] = {FAR_FSM_ERROR, &unexpect_id},
                        [FAR_TOK_WORD] = {FAR_FSM_ERROR, &unexpect_tok},
                        [FAR_TOK_EOF] = {FAR_FSM_END, &nop}},
    [FAR_FSM_SEQ_CONT] = {[FAR_TOK_NL] = {FAR_FSM_NL, &nop},
                          [FAR_TOK_ID] = {FAR_FSM_PAUSE, &store_id},
                          [FAR_TOK_WORD] = {FAR_FSM_SEQ_NL, &seq_cont},
                          [FAR_TOK_EOF] = {FAR_FSM_END, &nop}},
    [FAR_FSM_NL] = {[FAR_TOK_NL] = {FAR_FSM_NL, &nop},
                    [FAR_TOK_ID] = {FAR_FSM_PAUSE, &store_id},
                    [FAR_TOK_WORD] = {FAR_FSM_ERROR, &unexpect_tok},
                    [FAR_TOK_EOF] = {FAR_FSM_END, &nop}},
    [FAR_FSM_PAUSE] = {[FAR_TOK_NL] = {FAR_FSM_SEQ_BEGIN, &nop},
                       [FAR_TOK_ID] = {FAR_FSM_DESC_CONT, &desc_begin},
                       [FAR_TOK_WORD] = {FAR_FSM_DESC_CONT, &desc_begin},
                       [FAR_TOK_EOF] = {FAR_FSM_ERROR, &unexpect_eof}},
    [FAR_FSM_END] = {[FAR_TOK_NL] = {FAR_FSM_ERROR, &unexpect_nl},
                     [FAR_TOK_ID] = {FAR_FSM_ERROR, &unexpect_id},
                     [FAR_TOK_WORD] = {FAR_FSM_ERROR, &unexpect_tok},
                     [FAR_TOK_EOF] = {FAR_FSM_ERROR, &unexpect_eof}},
    [FAR_FSM_ERROR] = {[FAR_TOK_NL] = {FAR_FSM_ERROR, &nop},
                       [FAR_TOK_ID] = {FAR_FSM_ERROR, &nop},
                       [FAR_TOK_WORD] = {FAR_FSM_ERROR, &nop},
                       [FAR_TOK_EOF] = {FAR_FSM_ERROR, &nop}},
};

static char state_name[][14] = {
    [FAR_FSM_BEGIN] = "BEGIN",
    [FAR_FSM_DESC_BEGIN] = "DESC_BEGIN",
    [FAR_FSM_DESC_CONT] = "DESC_CONT",
    [FAR_FSM_SEQ_BEGIN] = "SEQ_BEGIN",
    [FAR_FSM_SEQ_NL] = "SEQ_NL",
    [FAR_FSM_SEQ_CONT] = "SEQ_CONT",
    [FAR_FSM_NL] = "NL",
    [FAR_FSM_PAUSE] = "PAUSE",
    [FAR_FSM_END] = "END",
    [FAR_FSM_ERROR] = "ERROR",
};

enum far_state fsm_next(enum far_state state, struct far_tok *tok,
                        struct far_aux *aux, struct far_tgt *tgt)
{
    unsigned row = (unsigned)state;
    unsigned col = (unsigned)tok->id;
    struct trans const *const t = &transition[row][col];
    struct args args = {tok, state, aux, tgt};
    if (t->action(&args)) return FAR_FSM_ERROR;
    return t->next;
}

char const *fsm_name(enum far_state state) { return state_name[state]; }

static enum far_rc read_desc(struct args *a)
{
    a->aux->pos = memccpy(a->aux->pos - 1, a->tok->value, '\0',
                          (unsigned long)(a->aux->end - a->aux->pos));
    if (!a->aux->pos) return error_parse(a->tok, "too long description");
    return FAR_SUCCESS;
}

static enum far_rc desc_begin(struct args *a)
{
    BUG(a->tok->id != FAR_TOK_WORD && a->tok->id != FAR_TOK_ID);
    a->aux->begin = a->tgt->desc;
    a->aux->pos = a->aux->begin + 1;
    a->aux->end = a->aux->begin + FAR_DESC_MAX;
    return read_desc(a);
}

static enum far_rc desc_cont(struct args *a)
{
    BUG(a->tok->id != FAR_TOK_WORD && a->tok->id != FAR_TOK_ID);
    *(a->aux->pos - 1) = ' ';
    a->aux->pos++;
    return read_desc(a);
}

static enum far_rc read_id(struct args *a)
{
    BUG(a->tok->id != FAR_TOK_ID);
    char const *ptr = memccpy(a->tgt->id, a->tok->value + 1, '\0', FAR_ID_MAX);
    if (!ptr) return error_parse(a->tok, "too long id");
    if (ptr - a->tgt->id == 1) return error_parse(a->tok, "empty id");
    return FAR_SUCCESS;
}

static enum far_rc seq_begin(struct args *a)
{
    BUG(a->tok->id != FAR_TOK_WORD);
    a->aux->begin = a->tgt->seq;
    a->aux->pos = a->aux->begin + 1;
    a->aux->end = a->aux->begin + FAR_SEQ_MAX;
    return seq_cont(a);
}

static enum far_rc seq_cont(struct args *a)
{
    BUG(a->tok->id != FAR_TOK_WORD);
    a->aux->pos = memccpy(a->aux->pos - 1, a->tok->value, '\0',
                          (unsigned long)(a->aux->end - a->aux->pos));
    if (!a->aux->pos) return error_parse(a->tok, "too long sequence");
    return FAR_SUCCESS;
}

static enum far_rc store_id(struct args *a)
{
    BUG(a->tok->id != FAR_TOK_ID);
    char const *ptr = memccpy(a->aux->id, a->tok->value + 1, '\0', FAR_ID_MAX);
    if (!ptr) return error_parse(a->tok, "too long id");
    if (ptr - a->aux->id == 1) return error_parse(a->tok, "empty id");
    return FAR_SUCCESS;
}
