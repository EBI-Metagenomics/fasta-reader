#ifndef FAR_PROF_H
#define FAR_PROF_H

#include "far/export.h"
#include "far/node.h"
#include "far/rc.h"
#include <stdio.h>

#define FAR_HEADER_MAX 64
#define FAR_NAME_MAX 64
#define FAR_ACC_MAX 64
#define FAR_DESC_MAX 128
#define FAR_LENG_MAX 8
#define FAR_ALPH_MAX 12
#define FAR_BUFF_MAX 128

struct far;

struct far_prof
{
    char header[FAR_HEADER_MAX];
    struct
    {
        char name[FAR_NAME_MAX];
        char acc[FAR_ACC_MAX];
        char desc[FAR_DESC_MAX];
        char leng[FAR_LENG_MAX];
        char alph[FAR_ALPH_MAX];
    } meta;
    char buff[FAR_BUFF_MAX];
    unsigned symbols_size;
    char symbols[FAR_SYMBOLS_MAX];

    struct far_node node;
    char *error;
};

#define FAR_PROF_DECLARE(name, far)                                            \
    struct far_prof name;                                                      \
    far_prof_init(&name, (far))

FAR_API void far_prof_dump(struct far_prof const *prof, FILE *restrict fd);
FAR_API void far_prof_init(struct far_prof *prof, struct far *far);
FAR_API enum far_rc far_prof_read(struct far_prof *prof);
FAR_API unsigned far_prof_length(struct far_prof const *prof);

#endif
