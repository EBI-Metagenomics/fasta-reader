#include "fsm.h"
#include "aux.h"
#include "bug.h"
#include "error.h"
#include "far/aux.h"
#include "far/prof.h"
#include "far/tok.h"
#include "far/trans.h"
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
    struct far_prof *prof;
};

struct trans
{
    enum far_state const next;
    enum far_rc (*action)(struct args *a);
};

static char const arrows[FAR_TRANS_SIZE][5] = {
    [FAR_TRANS_MM] = "m->m", [FAR_TRANS_MI] = "m->i", [FAR_TRANS_MD] = "m->d",
    [FAR_TRANS_IM] = "i->m", [FAR_TRANS_II] = "i->i", [FAR_TRANS_DM] = "d->m",
    [FAR_TRANS_DD] = "d->d"};

static enum far_rc nop(struct args *a) { return FAR_SUCCESS; }

static enum far_rc arrow(struct args *a);

static enum far_rc unexpect_eof(struct args *a)
{
    return error_parse(a->tok, "unexpected end-of-file");
}

static enum far_rc unexpect_eon(struct args *a)
{
    return error_parse(a->tok, "unexpected end-of-node");
}

static enum far_rc unexpect_symbol(struct args *a)
{
    return error_parse(a->tok, "unexpected symbol");
}

static enum far_rc unexpect_token(struct args *a)
{
    return error_parse(a->tok, "unexpected token");
}

static enum far_rc unexpect_newline(struct args *a)
{
    return error_parse(a->tok, "unexpected newline");
}

static enum far_rc header(struct args *a);

static enum far_rc field_name(struct args *a);

static enum far_rc field_content(struct args *a);

static enum far_rc hmm(struct args *a);

static enum far_rc symbol(struct args *a);

static enum far_rc compo(struct args *a);

static enum far_rc insert(struct args *a);

static enum far_rc match(struct args *a);

static enum far_rc trans(struct args *a);

static enum far_rc to_double(char const *str, double *val);

static enum far_rc to_lprob(char const *str, double *val);

static enum far_rc check_header(struct far_prof *prof);

static enum far_rc check_required_metadata(struct far_prof *prof);

static struct trans const transition[][6] = {
    [FAR_FSM_BEGIN] = {[FAR_TOK_WORD] = {FAR_FSM_HEADER, &header},
                       [FAR_TOK_NEWLINE] = {FAR_FSM_BEGIN, &nop},
                       [FAR_TOK_HMM] = {FAR_FSM_ERROR, &header},
                       [FAR_TOK_COMPO] = {FAR_FSM_ERROR, &header},
                       [FAR_TOK_SLASH] = {FAR_FSM_ERROR, &header},
                       [FAR_TOK_EOF] = {FAR_FSM_END, &nop}},
    [FAR_FSM_HEADER] = {[FAR_TOK_WORD] = {FAR_FSM_HEADER, &header},
                        [FAR_TOK_NEWLINE] = {FAR_FSM_NAME, &header},
                        [FAR_TOK_HMM] = {FAR_FSM_ERROR, &header},
                        [FAR_TOK_COMPO] = {FAR_FSM_ERROR, &header},
                        [FAR_TOK_SLASH] = {FAR_FSM_ERROR, &header},
                        [FAR_TOK_EOF] = {FAR_FSM_ERROR, &unexpect_eof}},
    [FAR_FSM_NAME] = {[FAR_TOK_WORD] = {FAR_FSM_CONTENT, &field_name},
                      [FAR_TOK_NEWLINE] = {FAR_FSM_ERROR, &unexpect_newline},
                      [FAR_TOK_HMM] = {FAR_FSM_SYMBOL, &hmm},
                      [FAR_TOK_COMPO] = {FAR_FSM_PAUSE, &nop},
                      [FAR_TOK_SLASH] = {FAR_FSM_ERROR, &unexpect_token},
                      [FAR_TOK_EOF] = {FAR_FSM_ERROR, &unexpect_eof}},
    [FAR_FSM_CONTENT] = {[FAR_TOK_WORD] = {FAR_FSM_CONTENT, &field_content},
                         [FAR_TOK_NEWLINE] = {FAR_FSM_NAME, &field_content},
                         [FAR_TOK_HMM] = {FAR_FSM_CONTENT, &field_content},
                         [FAR_TOK_COMPO] = {FAR_FSM_ERROR, &unexpect_token},
                         [FAR_TOK_SLASH] = {FAR_FSM_ERROR, &unexpect_token},
                         [FAR_TOK_EOF] = {FAR_FSM_ERROR, &unexpect_eof}},
    [FAR_FSM_SYMBOL] = {[FAR_TOK_WORD] = {FAR_FSM_SYMBOL, &symbol},
                        [FAR_TOK_NEWLINE] = {FAR_FSM_ARROW, &symbol},
                        [FAR_TOK_HMM] = {FAR_FSM_ERROR, &unexpect_symbol},
                        [FAR_TOK_COMPO] = {FAR_FSM_ERROR, &unexpect_symbol},
                        [FAR_TOK_SLASH] = {FAR_FSM_ERROR, &unexpect_symbol},
                        [FAR_TOK_EOF] = {FAR_FSM_ERROR, &unexpect_eof}},
    [FAR_FSM_ARROW] = {[FAR_TOK_WORD] = {FAR_FSM_ARROW, &arrow},
                       [FAR_TOK_NEWLINE] = {FAR_FSM_PAUSE, &arrow},
                       [FAR_TOK_HMM] = {FAR_FSM_ERROR, &unexpect_token},
                       [FAR_TOK_COMPO] = {FAR_FSM_ERROR, &unexpect_token},
                       [FAR_TOK_SLASH] = {FAR_FSM_ERROR, &unexpect_token},
                       [FAR_TOK_EOF] = {FAR_FSM_ERROR, &unexpect_eof}},
    [FAR_FSM_COMPO] = {[FAR_TOK_WORD] = {FAR_FSM_COMPO, &compo},
                       [FAR_TOK_NEWLINE] = {FAR_FSM_INSERT, &compo},
                       [FAR_TOK_HMM] = {FAR_FSM_ERROR, &unexpect_eon},
                       [FAR_TOK_COMPO] = {FAR_FSM_ERROR, &unexpect_eon},
                       [FAR_TOK_SLASH] = {FAR_FSM_ERROR, &unexpect_eon},
                       [FAR_TOK_EOF] = {FAR_FSM_ERROR, &unexpect_eof}},
    [FAR_FSM_INSERT] = {[FAR_TOK_WORD] = {FAR_FSM_INSERT, &insert},
                        [FAR_TOK_NEWLINE] = {FAR_FSM_TRANS, &insert},
                        [FAR_TOK_HMM] = {FAR_FSM_ERROR, &unexpect_eon},
                        [FAR_TOK_COMPO] = {FAR_FSM_ERROR, &unexpect_eon},
                        [FAR_TOK_SLASH] = {FAR_FSM_ERROR, &unexpect_eon},
                        [FAR_TOK_EOF] = {FAR_FSM_ERROR, &unexpect_eof}},
    [FAR_FSM_MATCH] = {[FAR_TOK_WORD] = {FAR_FSM_MATCH, &match},
                       [FAR_TOK_NEWLINE] = {FAR_FSM_INSERT, &match},
                       [FAR_TOK_HMM] = {FAR_FSM_ERROR, &unexpect_eon},
                       [FAR_TOK_COMPO] = {FAR_FSM_ERROR, &unexpect_eon},
                       [FAR_TOK_SLASH] = {FAR_FSM_ERROR, &unexpect_eon},
                       [FAR_TOK_EOF] = {FAR_FSM_ERROR, &unexpect_eof}},
    [FAR_FSM_TRANS] = {[FAR_TOK_WORD] = {FAR_FSM_TRANS, &trans},
                       [FAR_TOK_NEWLINE] = {FAR_FSM_PAUSE, &trans},
                       [FAR_TOK_HMM] = {FAR_FSM_ERROR, &unexpect_eon},
                       [FAR_TOK_COMPO] = {FAR_FSM_ERROR, &unexpect_eon},
                       [FAR_TOK_SLASH] = {FAR_FSM_ERROR, &unexpect_eon},
                       [FAR_TOK_EOF] = {FAR_FSM_ERROR, &unexpect_eof}},
    [FAR_FSM_PAUSE] = {[FAR_TOK_WORD] = {FAR_FSM_MATCH, &match},
                       [FAR_TOK_NEWLINE] = {FAR_FSM_ERROR, &unexpect_newline},
                       [FAR_TOK_HMM] = {FAR_FSM_ERROR, &unexpect_token},
                       [FAR_TOK_COMPO] = {FAR_FSM_COMPO, &nop},
                       [FAR_TOK_SLASH] = {FAR_FSM_SLASHED, &nop},
                       [FAR_TOK_EOF] = {FAR_FSM_ERROR, &unexpect_eof}},
    [FAR_FSM_SLASHED] = {[FAR_TOK_WORD] = {FAR_FSM_ERROR, &unexpect_token},
                         [FAR_TOK_NEWLINE] = {FAR_FSM_BEGIN, &nop},
                         [FAR_TOK_HMM] = {FAR_FSM_ERROR, &unexpect_token},
                         [FAR_TOK_COMPO] = {FAR_FSM_ERROR, &unexpect_token},
                         [FAR_TOK_SLASH] = {FAR_FSM_ERROR, &unexpect_token},
                         [FAR_TOK_EOF] = {FAR_FSM_ERROR, &unexpect_eof}},
};

static char state_name[][10] = {
    [FAR_FSM_BEGIN] = "BEGIN",   [FAR_FSM_HEADER] = "HEADER",
    [FAR_FSM_NAME] = "NAME",     [FAR_FSM_CONTENT] = "CONTENT",
    [FAR_FSM_SYMBOL] = "SYMBOL", [FAR_FSM_ARROW] = "ARROW",
    [FAR_FSM_COMPO] = "COMPO",   [FAR_FSM_INSERT] = "INSERT",
    [FAR_FSM_MATCH] = "MATCH",   [FAR_FSM_TRANS] = "TRANS",
    [FAR_FSM_PAUSE] = "PAUSE",   [FAR_FSM_SLASHED] = "SLASHED",
    [FAR_FSM_END] = "END",       [FAR_FSM_ERROR] = "ERROR",
};

enum far_state fsm_next(enum far_state state, struct far_tok *tok,
                        struct far_aux *aux, struct far_prof *prof)
{
    unsigned row = (unsigned)state;
    unsigned col = (unsigned)tok->id;
    struct trans const *const t = &transition[row][col];
    struct args args = {tok, state, aux, prof};
    if (t->action(&args)) return FAR_FSM_ERROR;
    return t->next;
}

char const *fsm_name(enum far_state state) { return state_name[state]; }

static enum far_rc arrow(struct args *a)
{
    BUG(a->tok->id != FAR_TOK_WORD && a->tok->id != FAR_TOK_NEWLINE);
    if (a->tok->id == FAR_TOK_WORD)
    {
        if (a->aux->idx >= FAR_TRANS_SIZE) return unexpect_token(a);

        if (strcmp(a->tok->value, arrows[a->aux->idx]))
            return error_parse(a->tok, "expected %s", arrows[a->aux->idx]);
        a->aux->idx++;
    }
    else
    {
        if (a->aux->idx != FAR_TRANS_SIZE)
            return error_parse(a->tok, "unexpected end-of-line");
        aux_init(a->aux);
    }
    return FAR_SUCCESS;
}

static enum far_rc header(struct args *a)
{
    BUG(a->tok->id != FAR_TOK_WORD && a->tok->id != FAR_TOK_NEWLINE);
    if (a->tok->id == FAR_TOK_WORD)
    {
        if (a->aux->prof.pos > a->aux->prof.begin + 1)
        {
            *(a->aux->prof.pos - 1) = ' ';
            a->aux->prof.pos++;
        }
        else
        {
            a->aux->prof.begin = a->prof->header;
            a->aux->prof.pos = a->aux->prof.begin + 1;
            a->aux->prof.end = a->aux->prof.begin + FAR_HEADER_MAX;
        }
        a->aux->prof.pos =
            memccpy(a->aux->prof.pos - 1, a->tok->value, '\0',
                    (unsigned long)(a->aux->prof.end - a->aux->prof.pos));
    }
    else
    {
        *(a->aux->prof.pos - 1) = '\0';
        if (check_header(a->prof)) return error_parse(a->tok, "invalid header");
        aux_init(a->aux);
    }
    return FAR_SUCCESS;
}

static enum far_rc field_name(struct args *a)
{
    if (!strcmp(a->tok->value, "NAME"))
    {
        a->aux->prof.begin = a->prof->meta.name;
        a->aux->prof.end = a->aux->prof.begin + FAR_NAME_MAX;
    }
    else if (!strcmp(a->tok->value, "ACC"))
    {
        a->aux->prof.begin = a->prof->meta.acc;
        a->aux->prof.end = a->aux->prof.begin + FAR_ACC_MAX;
    }
    else if (!strcmp(a->tok->value, "DESC"))
    {
        a->aux->prof.begin = a->prof->meta.desc;
        a->aux->prof.end = a->aux->prof.begin + FAR_DESC_MAX;
    }
    else if (!strcmp(a->tok->value, "LENG"))
    {
        a->aux->prof.begin = a->prof->meta.leng;
        a->aux->prof.end = a->aux->prof.begin + FAR_LENG_MAX;
    }
    else if (!strcmp(a->tok->value, "ALPH"))
    {
        a->aux->prof.begin = a->prof->meta.alph;
        a->aux->prof.end = a->aux->prof.begin + FAR_ALPH_MAX;
    }
    else
    {
        a->aux->prof.begin = a->prof->buff;
        a->aux->prof.end = a->aux->prof.begin + FAR_BUFF_MAX;
    }
    a->aux->prof.pos = a->aux->prof.begin + 1;
    return FAR_SUCCESS;
}

static enum far_rc field_content(struct args *a)
{
    BUG(a->tok->id != FAR_TOK_WORD && a->tok->id != FAR_TOK_HMM &&
        a->tok->id != FAR_TOK_COMPO && a->tok->id != FAR_TOK_NEWLINE);

    if (a->tok->id == FAR_TOK_WORD || a->tok->id == FAR_TOK_HMM ||
        a->tok->id == FAR_TOK_COMPO)
    {
        if (a->aux->prof.pos > a->aux->prof.begin + 1)
        {
            *(a->aux->prof.pos - 1) = ' ';
            a->aux->prof.pos++;
        }
        a->aux->prof.pos =
            memccpy(a->aux->prof.pos - 1, a->tok->value, '\0',
                    (unsigned long)(a->aux->prof.end - a->aux->prof.pos));
    }
    else
    {
        if (a->aux->prof.pos == a->aux->prof.begin + 1)
            return error_parse(a->tok, "expected content before end-of-line");
        *(a->aux->prof.pos - 1) = '\0';
        aux_init(a->aux);
    }
    return FAR_SUCCESS;
}

static enum far_rc hmm(struct args *a)
{
    a->aux->prof.begin = a->prof->symbols;
    a->aux->prof.end = a->aux->prof.begin + FAR_SYMBOLS_MAX;
    a->aux->prof.pos = a->aux->prof.begin + 1;
    return check_required_metadata(a->prof);
}

static enum far_rc symbol(struct args *a)
{
    BUG(a->tok->id != FAR_TOK_WORD && a->tok->id != FAR_TOK_NEWLINE);
    if (a->tok->id == FAR_TOK_WORD)
    {
        *(a->aux->prof.pos - 1) = *a->tok->value;
        a->aux->prof.pos++;
    }
    else
    {
        *(a->aux->prof.pos - 1) = '\0';
        a->prof->symbols_size = (unsigned)strlen(a->prof->symbols);
        a->prof->node.symbols_size = a->prof->symbols_size;
        aux_init(a->aux);
    }
    return FAR_SUCCESS;
}

static enum far_rc compo(struct args *a)
{
    BUG(a->tok->id != FAR_TOK_WORD && a->tok->id != FAR_TOK_NEWLINE);
    if (a->tok->id == FAR_TOK_WORD)
    {
        if (a->aux->idx >= a->prof->symbols_size)
            return error_parse(a->tok, "too many compo numbers");

        if (to_lprob(a->tok->value, a->prof->node.compo + a->aux->idx++))
            return error_parse(a->tok, DEC_ERROR);
    }
    else
    {
        if (a->aux->idx != a->prof->symbols_size)
            return error_parse(a->tok,
                               "compo length not equal to symbols length");
        aux_init(a->aux);
    }
    return FAR_SUCCESS;
}

static enum far_rc insert(struct args *a)
{
    BUG(a->tok->id != FAR_TOK_WORD && a->tok->id != FAR_TOK_NEWLINE);
    if (a->tok->id == FAR_TOK_WORD)
    {
        if (a->aux->idx >= a->prof->symbols_size)
            return error_parse(a->tok, "too many insert numbers");

        if (to_lprob(a->tok->value, a->prof->node.insert + a->aux->idx++))
            return error_parse(a->tok, DEC_ERROR);
    }
    else
    {
        if (a->aux->idx != a->prof->symbols_size)
            return error_parse(a->tok,
                               "insert length not equal to symbols length");
        aux_init(a->aux);
    }
    return FAR_SUCCESS;
}

static enum far_rc match(struct args *a)
{
    BUG(a->tok->id != FAR_TOK_WORD && a->tok->id != FAR_TOK_NEWLINE);
    if (a->tok->id == FAR_TOK_WORD)
    {
        if (a->state == FAR_FSM_PAUSE)
        {
            unsigned i = (unsigned)strtoul(a->tok->value, NULL, 10);
            if (i == 0) return error_parse(a->tok, "failed to convert integer");

            a->prof->node.idx = i;
            return FAR_SUCCESS;
        }
        if (a->aux->idx >= a->prof->symbols_size)
        {
            if (a->aux->idx >= a->prof->symbols_size + FAR_MATCH_EXCESS_SIZE)
                return error_parse(a->tok, "too many match numbers");

            a->aux->idx++;
            return FAR_SUCCESS;
        }
        if (to_lprob(a->tok->value, a->prof->node.match + a->aux->idx++))
            return error_parse(a->tok, DEC_ERROR);
    }
    else
    {
        if (a->aux->idx > a->prof->symbols_size + FAR_MATCH_EXCESS_SIZE)
            return error_parse(a->tok, "too many match numbers");
        aux_init(a->aux);
    }
    return FAR_SUCCESS;
}

static enum far_rc trans(struct args *a)
{
    BUG(a->tok->id != FAR_TOK_WORD && a->tok->id != FAR_TOK_NEWLINE);
    if (a->tok->id == FAR_TOK_WORD)
    {
        if (a->aux->idx >= FAR_TRANS_SIZE)
            return error_parse(a->tok, "too many trans numbers");

        if (to_lprob(a->tok->value, a->prof->node.trans + a->aux->idx++))
            return error_parse(a->tok, DEC_ERROR);
    }
    else
    {
        if (a->aux->idx != FAR_TRANS_SIZE)
            return error_parse(
                a->tok, "trans length not equal to " XSTR(FAR_TRANS_SIZE));
        aux_init(a->aux);
    }
    return FAR_SUCCESS;
}

static enum far_rc to_double(char const *str, double *val)
{
    if (str[0] == '*' && str[1] == '\0')
    {
        *val = INFINITY;
        return FAR_SUCCESS;
    }
    char *ptr = NULL;
    *val = strtod(str, &ptr);

    if (*val == 0.0 && str == ptr) return FAR_PARSEERROR;

    if (strchr(str, '\0') != ptr) return FAR_PARSEERROR;

    return FAR_SUCCESS;
}

static enum far_rc to_lprob(char const *str, double *val)
{
    enum far_rc rc = to_double(str, val);
    if (rc) return rc;
    *val = -(*val);
    return FAR_SUCCESS;
}

#define HEADER_EXAMPLE "HMMER3/f [3.3.1 | Jul 2020]"
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

static enum far_rc check_header(struct far_prof *prof)
{
    char tmp[sizeof HEADER_EXAMPLE + 10];
    if (strlen(prof->header) >= ARRAY_SIZE(tmp)) return FAR_PARSEERROR;

    strcpy(tmp, prof->header);
    char *ptr = NULL;
    char *tok = NULL;

    if (!(tok = strtok_r(tmp, " ", &ptr))) return FAR_PARSEERROR;

    if (strcmp(tok, "HMMER3/f")) return FAR_PARSEERROR;

    if (!(tok = strtok_r(NULL, " ", &ptr))) return FAR_PARSEERROR;

    if (*tok != '[') return FAR_PARSEERROR;

    if (!(tok = strtok_r(NULL, " ", &ptr))) return FAR_PARSEERROR;

    if (*tok != '|') return FAR_PARSEERROR;

    /* Month */
    if (!(tok = strtok_r(NULL, " ", &ptr))) return FAR_PARSEERROR;

    /* Year] */
    if (!(tok = strtok_r(NULL, " ", &ptr))) return FAR_PARSEERROR;

    if (!(tok = strchr(tok, ']'))) return FAR_PARSEERROR;

    if (strtok_r(NULL, " ", &ptr)) return FAR_PARSEERROR;

    return FAR_SUCCESS;
}

static enum far_rc check_required_metadata(struct far_prof *prof)
{
    if (prof->meta.acc[0] == '\0')
        return error_parse(prof, "missing ACC field");

    if (prof->meta.desc[0] == '\0')
        return error_parse(prof, "missing DESC field");

    if (prof->meta.leng[0] == '\0')
        return error_parse(prof, "missing LENG field");

    if (prof->meta.alph[0] == '\0')
        return error_parse(prof, "missing ALPH field");

    return FAR_SUCCESS;
}
