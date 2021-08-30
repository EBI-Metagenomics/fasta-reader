#include "far/far.h"
#include "fsm.h"
#include "prof.h"
#include "tok.h"

void far_init(struct far *far, FILE *restrict fd)
{
    far->fd = fd;
    fsm_init(&far->state);
    far->error[0] = '\0';
    tok_init(&far->tok, far->error);
}

enum far_rc far_next_prof(struct far *far, struct far_prof *prof)
{
    return prof_next_prof(prof, far->fd, &far->aux, &far->state, &far->tok);
}

enum far_rc far_next_node(struct far *far, struct far_prof *prof)
{
    return prof_next_node(prof, far->fd, &far->aux, &far->state, &far->tok);
}

void far_clear_error(struct far *far) { far->error[0] = '\0'; }
