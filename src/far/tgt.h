#ifndef FAR_TGT_H
#define FAR_TGT_H

#include "far/export.h"
#include "far/rc.h"
#include <stdio.h>

#define FAR_ID_MAX 64
#define FAR_DESC_MAX 128
#define FAR_SEQ_MAX (1024 * 1024)

struct fasta;

struct fasta_target
{
    char *id;
    char *desc;
    char *seq;
    char *error;

    struct
    {
        char id[FAR_ID_MAX];
        char desc[FAR_DESC_MAX];
        char seq[FAR_SEQ_MAX];
    } buf;
};

FAR_API void far_tgt_write(char const *id, char const *desc, char const *seq,
                           unsigned ncols, FILE *restrict fd);
FAR_API void fasta_target_init(struct fasta_target *tgt, struct fasta *far);

#endif
