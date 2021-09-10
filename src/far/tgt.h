#ifndef FAR_TGT_H
#define FAR_TGT_H

#include "far/export.h"
#include "far/rc.h"
#include <stdio.h>

#define FAR_ID_MAX 64
#define FAR_DESC_MAX 128
#define FAR_SEQ_MAX (1024 * 1024)

struct far;

struct far_tgt
{
    char id[FAR_ID_MAX];
    char desc[FAR_DESC_MAX];
    char seq[FAR_SEQ_MAX];
    char *error;
};

#define FAR_TGT_DECLARE(name, far)                                             \
    struct far_tgt name;                                                       \
    far_tgt_init(&name, (far))

FAR_API void far_tgt_dump(struct far_tgt const *tgt, FILE *restrict fd);
FAR_API void far_tgt_init(struct far_tgt *tgt, struct far *far);

#endif
