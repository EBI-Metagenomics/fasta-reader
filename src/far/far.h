#ifndef FAR_FAR_H
#define FAR_FAR_H

#include "far/aux.h"
#include "far/error.h"
#include "far/export.h"
#include "far/rc.h"
#include "far/tgt.h"
#include "far/tok.h"
#include <stdio.h>

enum fasta_mode
{
    FASTA_READ,
    FASTA_WRITE,
};

struct fasta
{
    FILE *restrict fd;
    enum fasta_mode mode;
    unsigned state;
    struct fasta_tok tok;
    struct fasta_aux aux;
    char error[FAR_ERROR_SIZE];
};

FAR_API void fasta_init(struct fasta *fa, FILE *restrict fd,
                        enum fasta_mode mode);
FAR_API enum fasta_rc fasta_next_target(struct fasta *fa,
                                        struct fasta_target *tgt);
FAR_API void fasta_clear_error(struct fasta *fa);

#endif
