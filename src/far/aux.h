#ifndef FAR_AUX_H
#define FAR_AUX_H

#include "tgt.h"

struct fasta_aux
{
    char *begin;
    char *pos;
    char *end;
    char id[FAR_ID_MAX];
};

#endif
