#ifndef FAR_FAR_H
#define FAR_FAR_H

#include "far/aux.h"
#include "far/error.h"
#include "far/export.h"
#include "far/rc.h"
#include "far/state.h"
#include "far/tgt.h"
#include "far/tok.h"
#include <stdio.h>

struct far
{
    FILE *restrict fd;
    enum far_state state;
    struct far_tok tok;
    struct far_aux aux;
    char error[FAR_ERROR_SIZE];
};

#define FAR_DECLARE(name, fd)                                                  \
    struct far name;                                                           \
    far_init(&name, fd)

FAR_API void far_init(struct far *far, FILE *restrict fd);
FAR_API enum far_rc far_next_tgt(struct far *far, struct far_tgt *prof);
FAR_API void far_clear_error(struct far *far);

#endif
